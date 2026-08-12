use std::cell::RefCell;
use std::collections::{HashMap, HashSet};
use std::fmt;
use std::fs::{self, File, OpenOptions};
use std::io::{BufRead, Read, Seek, SeekFrom, Write};
use std::ops::{Deref, DerefMut};
use std::path::{Component, Path, PathBuf};
use std::rc::{Rc, Weak};
use std::sync::atomic::{AtomicI64, Ordering};
use std::time::{SystemTime, UNIX_EPOCH};

#[cfg(unix)]
use std::os::unix::ffi::{OsStrExt, OsStringExt};

use crate::ast::{Argument, Binding, Expr, ModulePart, Stmt};
use crate::ir::{Constant as IrConstant, LegacyIr, Opcode as IrOpcode, Operand as IrOperand};
use crate::lexer::symbol_to_bytes;
use crate::{Location, ParseError, Parser};

type Cell = Rc<RefCell<ValueSlot>>;
type Environment = Rc<RefCell<Env>>;
type ConstantPool = Rc<RefCell<HashMap<Vec<u8>, Cell>>>;
type RuntimeList = Rc<RefCell<Vec<ListEntry>>>;

const IDENTITY_BASE: i64 = 100_000_000_000_000;
static NEXT_IDENTITY: AtomicI64 = AtomicI64::new(IDENTITY_BASE + 4);

struct Identity(i64);

#[derive(Clone)]
struct ValueSlot {
    value: Value,
    identity: Rc<Identity>,
}

impl ValueSlot {
    fn new(value: Value) -> Self {
        Self {
            value,
            identity: Rc::new(Identity(NEXT_IDENTITY.fetch_add(1, Ordering::Relaxed))),
        }
    }
}

impl Deref for ValueSlot {
    type Target = Value;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}

impl DerefMut for ValueSlot {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.value
    }
}

#[derive(Clone)]
struct IrFunction {
    code: Rc<LegacyIr>,
    base: usize,
    parameter_count: usize,
    closure: Environment,
    source: String,
    source_bytes: Vec<u8>,
    constants: ConstantPool,
}

#[derive(Clone)]
struct IrThunk {
    code: Rc<LegacyIr>,
    base: usize,
    environment: Environment,
    cached: Rc<RefCell<Option<Cell>>>,
    source: String,
    source_bytes: Vec<u8>,
    constants: ConstantPool,
}

#[derive(Clone)]
enum VmContextKind {
    Root,
    Function,
    Thunk(IrThunk),
}

#[derive(Clone)]
struct VmContext {
    pre_context: Option<Rc<RefCell<VmContext>>>,
    live_context: Weak<RefCell<VmContext>>,
    code: Rc<RefCell<LegacyIr>>,
    pc: usize,
    environment: Environment,
    stack: Rc<RefCell<Vec<Cell>>>,
    call_anchors: Vec<usize>,
    source: String,
    source_bytes: Vec<u8>,
    constants: ConstantPool,
    trace: Vec<(Vec<u8>, Location)>,
    kind: VmContextKind,
}

struct VmOutcome {
    value: Cell,
    environment: Environment,
    live_context: Rc<RefCell<VmContext>>,
    root_returned: bool,
}

struct IncrementalVmOutcome {
    ir: LegacyIr,
    environment: Environment,
    root_return: Option<Cell>,
}

struct LoadedModule {
    value: Cell,
    root_return: Option<Cell>,
    deferred_cache_key: Option<Vec<u8>>,
}

#[derive(Clone)]
struct Class {
    members: Environment,
}

struct RuntimeFile {
    file: Option<File>,
    read_buffer: Vec<u8>,
    read_cursor: usize,
    write_buffer: Vec<u8>,
    good: bool,
    eof: bool,
    fail: bool,
}

impl RuntimeFile {
    const WRITE_BUFFER_CAPACITY: usize = 8_191;

    fn flush_write_buffer(&mut self) -> std::io::Result<()> {
        if self.write_buffer.is_empty() {
            return Ok(());
        }
        let file = self.file.as_mut().ok_or_else(|| {
            std::io::Error::new(std::io::ErrorKind::NotConnected, "file is closed")
        })?;
        file.write_all(&self.write_buffer)?;
        self.write_buffer.clear();
        Ok(())
    }

    fn logical_position(&mut self) -> std::io::Result<u64> {
        let buffered = self.write_buffer.len() as u64;
        let unread = self.read_buffer.len().saturating_sub(self.read_cursor) as u64;
        self.file
            .as_mut()
            .ok_or_else(|| std::io::Error::new(std::io::ErrorKind::NotConnected, "file is closed"))?
            .stream_position()
            .map(|position| position.saturating_add(buffered).saturating_sub(unread))
    }

    fn write_buffered(&mut self, bytes: &[u8]) -> std::io::Result<()> {
        let unread = self.read_buffer.len().saturating_sub(self.read_cursor);
        if unread != 0 {
            self.file
                .as_mut()
                .ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::NotConnected, "file is closed")
                })?
                .seek(SeekFrom::Current(-(unread as i64)))?;
        }
        self.read_buffer.clear();
        self.read_cursor = 0;
        let available = Self::WRITE_BUFFER_CAPACITY - self.write_buffer.len();
        if bytes.len() >= available {
            self.flush_write_buffer()?;
            self.file
                .as_mut()
                .ok_or_else(|| {
                    std::io::Error::new(std::io::ErrorKind::NotConnected, "file is closed")
                })?
                .write_all(bytes)
        } else {
            self.write_buffer.extend_from_slice(bytes);
            Ok(())
        }
    }

    fn read_buffered_byte(&mut self) -> std::io::Result<Option<u8>> {
        if let Some(byte) = self.read_buffer.get(self.read_cursor).copied() {
            self.read_cursor += 1;
            return Ok(Some(byte));
        }
        self.read_buffer.clear();
        self.read_cursor = 0;
        self.flush_write_buffer()?;
        let mut buffer = vec![0; Self::WRITE_BUFFER_CAPACITY];
        let count = self
            .file
            .as_mut()
            .ok_or_else(|| std::io::Error::new(std::io::ErrorKind::NotConnected, "file is closed"))?
            .read(&mut buffer)?;
        if count == 0 {
            return Ok(None);
        }
        buffer.truncate(count);
        let byte = buffer[0];
        self.read_buffer = buffer;
        self.read_cursor = 1;
        Ok(Some(byte))
    }

    fn discard_read_buffer(&mut self) {
        self.read_buffer.clear();
        self.read_cursor = 0;
    }
}

impl Drop for RuntimeFile {
    fn drop(&mut self) {
        let _ = self.flush_write_buffer();
    }
}

#[derive(Clone)]
struct DictEntry {
    key: Cell,
    value: Cell,
    order_key: Vec<u8>,
}

#[derive(Clone)]
struct ListEntry {
    node: Rc<()>,
    value: Cell,
}

impl ListEntry {
    fn new(value: Cell) -> Self {
        Self {
            node: Rc::new(()),
            value,
        }
    }
}

#[derive(Clone)]
struct ListIterator {
    list: RuntimeList,
    next: Rc<RefCell<Option<Rc<()>>>>,
}

#[derive(Clone, Copy)]
enum Builtin {
    Print,
    Println,
    Str,
    Type,
    Time,
    Eval,
    Id,
    Args,
    Open,
    CurrentPath,
    IsDirectory,
    ReadDirectory,
    CallWithCurrentContinuation,
    Input,
    Exit,
}

#[derive(Clone)]
enum Value {
    Unbound(String),
    None,
    Bool(bool),
    Integer(i64),
    Float(f64),
    String(Vec<u8>),
    List(RuntimeList),
    ListIterator(Rc<ListIterator>),
    Dict(Rc<RefCell<Vec<DictEntry>>>),
    File(Rc<RefCell<RuntimeFile>>),
    Path(PathBuf),
    Continuation(Rc<VmContext>),
    IrFunction(Rc<IrFunction>),
    Namespace(Environment),
    Enum(Environment),
    Class(Rc<Class>),
    Instance {
        class: Rc<Class>,
        fields: Environment,
    },
    UserIrMethod {
        function: Rc<IrFunction>,
        receiver: Cell,
    },
    Builtin(Builtin),
    BoundMethod {
        receiver: Cell,
        name: String,
    },
    IrThunk(IrThunk),
}

struct Env {
    values: HashMap<String, Cell>,
    parent: Option<Environment>,
    fresh_loads: bool,
}

impl Env {
    fn root() -> Environment {
        Rc::new(RefCell::new(Self {
            values: HashMap::new(),
            parent: None,
            fresh_loads: false,
        }))
    }

    fn builtin_root() -> Environment {
        Rc::new(RefCell::new(Self {
            values: HashMap::new(),
            parent: None,
            fresh_loads: true,
        }))
    }

    fn child(parent: &Environment) -> Environment {
        Rc::new(RefCell::new(Self {
            values: HashMap::new(),
            parent: Some(Rc::clone(parent)),
            fresh_loads: false,
        }))
    }

    fn find_entry(environment: &Environment, name: &str) -> Option<(Cell, bool)> {
        let (value, parent, fresh_loads) = {
            let borrowed = environment.borrow();
            (
                borrowed.values.get(name).cloned(),
                borrowed.parent.clone(),
                borrowed.fresh_loads,
            )
        };
        value
            .map(|value| (value, fresh_loads))
            .or_else(|| parent.and_then(|parent| Self::find_entry(&parent, name)))
    }

    fn find(environment: &Environment, name: &str) -> Option<Cell> {
        Self::find_entry(environment, name).map(|(value, _)| value)
    }

    fn load(environment: &Environment, name: &str) -> Cell {
        if let Some((value, fresh_loads)) = Self::find_entry(environment, name) {
            let value = if fresh_loads {
                copy_variable(value)
            } else {
                value
            };
            name_unbound(&value, name);
            return value;
        }
        let value = cell(Value::Unbound(name.to_owned()));
        Self::define(environment, name.to_owned(), value.clone());
        value
    }

    fn find_local(environment: &Environment, name: &str) -> Option<Cell> {
        environment.borrow().values.get(name).cloned()
    }

    fn define(environment: &Environment, name: String, value: Cell) {
        environment.borrow_mut().values.insert(name, value);
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct RuntimeError {
    pub location: Option<Location>,
    pub message: String,
    details: Box<RuntimeErrorDetails>,
}

#[derive(Clone, Debug, Default, PartialEq)]
struct RuntimeErrorDetails {
    message_bytes: Option<Vec<u8>>,
    source: Option<String>,
    source_bytes: Option<Vec<u8>>,
    trace: Vec<(Vec<u8>, Location)>,
    diagnostic: Option<String>,
    diagnostic_bytes: Option<Vec<u8>>,
}

impl RuntimeError {
    fn new(message: impl Into<String>) -> Self {
        Self {
            location: None,
            message: message.into(),
            details: Box::default(),
        }
    }

    fn at(location: Location, message: impl Into<String>) -> Self {
        Self {
            location: Some(location),
            message: message.into(),
            details: Box::default(),
        }
    }

    fn plain_diagnostic(message: impl Into<String>) -> Self {
        let message = message.into();
        let diagnostic = message.clone();
        Self {
            location: None,
            message,
            details: Box::new(RuntimeErrorDetails {
                diagnostic: Some(diagnostic),
                ..RuntimeErrorDetails::default()
            }),
        }
    }

    fn plain_diagnostic_bytes(message: Vec<u8>) -> Self {
        Self {
            location: None,
            message: String::from_utf8_lossy(&message).into_owned(),
            details: Box::new(RuntimeErrorDetails {
                diagnostic: Some(String::from_utf8_lossy(&message).into_owned()),
                diagnostic_bytes: Some(message),
                ..RuntimeErrorDetails::default()
            }),
        }
    }

    fn replace_diagnostic_source(mut self, displayed: &str, raw: &[u8]) -> Self {
        if let Some(diagnostic) = &self.details.diagnostic {
            let bytes = self
                .details
                .diagnostic_bytes
                .as_deref()
                .unwrap_or(diagnostic.as_bytes());
            if let Some(index) = bytes
                .windows(displayed.len())
                .position(|window| window == displayed.as_bytes())
            {
                let mut replaced =
                    Vec::with_capacity(bytes.len() + raw.len().saturating_sub(displayed.len()));
                replaced.extend_from_slice(&bytes[..index]);
                replaced.extend_from_slice(raw);
                replaced.extend_from_slice(&bytes[index + displayed.len()..]);
                self.details.diagnostic_bytes = Some(replaced);
            }
        }
        self
    }

    fn new_bytes(message: Vec<u8>) -> Self {
        Self {
            location: None,
            message: String::from_utf8_lossy(&message).into_owned(),
            details: Box::new(RuntimeErrorDetails {
                message_bytes: Some(message),
                ..RuntimeErrorDetails::default()
            }),
        }
    }

    #[must_use]
    pub fn render_bytes(&self) -> Vec<u8> {
        if let Some(diagnostic) = &self.details.diagnostic_bytes {
            return diagnostic.clone();
        }
        if let Some(diagnostic) = &self.details.diagnostic {
            return diagnostic.as_bytes().to_vec();
        }
        let message = self
            .details
            .message_bytes
            .as_deref()
            .unwrap_or(self.message.as_bytes());
        if self.details.source.is_some() {
            let mut rendered = b"\x1b[1mTraceback:\n\x1b[0m".to_vec();
            for (source, location) in self.details.trace.iter().take(66) {
                let column = if location.line == 0 {
                    location.column
                } else {
                    location.column + 1
                };
                rendered.extend(b"  running at ");
                rendered.extend(source);
                rendered.extend(format!(":{}:{column}\n", location.line).as_bytes());
            }
            if let Some(location) = self.location {
                let column = if location.line == 0 {
                    location.column
                } else {
                    location.column + 1
                };
                rendered.extend(b"\x1b[1m  running at ");
                if let Some(source) = &self.details.source_bytes {
                    rendered.extend(source);
                } else if let Some(source) = &self.details.source {
                    rendered.extend(source.as_bytes());
                }
                rendered.extend(format!(":{}:{column}: \x1b[0m", location.line).as_bytes());
            }
            rendered.extend(b"\x1b[31merror: \x1b[0m");
            rendered.extend(message);
            return rendered;
        }
        let mut rendered = match self.location {
            Some(location) => {
                format!("{}:{}: error: ", location.line, location.column + 1).into_bytes()
            }
            None => b"error: ".to_vec(),
        };
        rendered.extend(message);
        rendered
    }
}

impl RuntimeError {
    fn has_source_or_diagnostic(&self) -> bool {
        self.details.source.is_some() || self.details.diagnostic.is_some()
    }

    fn has_diagnostic(&self) -> bool {
        self.details.diagnostic.is_some()
    }

    fn attach_source(
        &mut self,
        source: String,
        source_bytes: Vec<u8>,
        trace: Vec<(Vec<u8>, Location)>,
    ) {
        self.details.source = Some(source);
        self.details.source_bytes = Some(source_bytes);
        self.details.trace = trace;
    }
}

impl fmt::Display for RuntimeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", String::from_utf8_lossy(&self.render_bytes()))
    }
}

impl std::error::Error for RuntimeError {}

impl From<ParseError> for RuntimeError {
    fn from(error: ParseError) -> Self {
        let diagnostic_bytes = error.render_bytes();
        let diagnostic = String::from_utf8_lossy(&diagnostic_bytes).into_owned();
        Self {
            location: Some(error.location),
            message: error.message,
            details: Box::new(RuntimeErrorDetails {
                diagnostic: Some(diagnostic),
                diagnostic_bytes: Some(diagnostic_bytes),
                ..RuntimeErrorDetails::default()
            }),
        }
    }
}

pub struct Interpreter {
    builtins: Environment,
    globals: Environment,
    output: String,
    stream_output: bool,
    arguments: Vec<Vec<u8>>,
    directories: Vec<PathBuf>,
    modules: HashMap<Vec<u8>, Cell>,
    halted: bool,
    prefix_operators: HashSet<String>,
    infix_operators: Vec<(String, u64)>,
    current_source: String,
    current_source_bytes: Vec<u8>,
    current_constants: ConstantPool,
    call_frames: Vec<(Vec<u8>, Location)>,
    repl_code: Rc<RefCell<LegacyIr>>,
    repl_live_context: Option<Rc<RefCell<VmContext>>>,
    last_file_ir: Option<LegacyIr>,
}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}

impl Interpreter {
    #[must_use]
    pub fn new() -> Self {
        let builtins = Env::builtin_root();
        for (name, builtin) in [
            ("print", Builtin::Print),
            ("println", Builtin::Println),
            ("str", Builtin::Str),
            ("type", Builtin::Type),
            ("time", Builtin::Time),
            ("eval", Builtin::Eval),
            ("id", Builtin::Id),
            ("input", Builtin::Input),
            ("exit", Builtin::Exit),
            (
                "call_with_current_continuation",
                Builtin::CallWithCurrentContinuation,
            ),
        ] {
            Env::define(&builtins, name.to_owned(), cell(Value::Builtin(builtin)));
        }
        let globals = Env::child(&builtins);
        Self {
            builtins,
            globals,
            output: String::new(),
            stream_output: false,
            arguments: Vec::new(),
            directories: vec![std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."))],
            modules: HashMap::new(),
            halted: false,
            prefix_operators: HashSet::new(),
            infix_operators: Vec::new(),
            current_source: "<stdin>".to_owned(),
            current_source_bytes: b"<stdin>".to_vec(),
            current_constants: Rc::new(RefCell::new(HashMap::new())),
            call_frames: Vec::new(),
            repl_code: Rc::new(RefCell::new(LegacyIr::default())),
            repl_live_context: None,
            last_file_ir: None,
        }
    }

    #[must_use]
    pub fn with_arguments(arguments: Vec<String>) -> Self {
        let mut interpreter = Self::new();
        interpreter.arguments = arguments.into_iter().map(String::into_bytes).collect();
        interpreter
    }

    #[must_use]
    pub fn with_os_arguments(arguments: Vec<std::ffi::OsString>) -> Self {
        let mut interpreter = Self::new();
        #[cfg(unix)]
        {
            interpreter.arguments = arguments
                .into_iter()
                .map(std::ffi::OsString::into_vec)
                .collect();
        }
        #[cfg(not(unix))]
        {
            interpreter.arguments = arguments
                .into_iter()
                .map(|argument| argument.to_string_lossy().as_bytes().to_vec())
                .collect();
        }
        interpreter
    }

    pub fn set_stream_output(&mut self, enabled: bool) {
        self.stream_output = enabled;
    }

    pub fn run(&mut self, source: &str, name: &str) -> Result<String, RuntimeError> {
        self.run_internal(source.as_bytes(), name, None, None, false, None)
    }

    pub fn run_file(&mut self, source: &str, path: &Path) -> Result<String, RuntimeError> {
        self.run_file_bytes(source.as_bytes(), path)
    }

    pub fn run_file_bytes(&mut self, source: &[u8], path: &Path) -> Result<String, RuntimeError> {
        let ir_path = sidecar_path(path, ".ir");
        let source_name_bytes = path
            .file_name()
            .map(|name| path_bytes(Path::new(name)))
            .unwrap_or_else(|| path_bytes(path));
        self.run_internal(
            source,
            &path.display().to_string(),
            Some(source_name_bytes),
            Some(path_bytes(path)),
            false,
            Some(&ir_path),
        )
    }

    pub fn write_debug_ir(
        &self,
        source_path: &Path,
        output_path: &Path,
    ) -> Result<(), RuntimeError> {
        let cached;
        let ir = if let Some(ir) = &self.last_file_ir {
            ir
        } else {
            let ir_path = sidecar_path(source_path, ".ir");
            cached = LegacyIr::read_from(&ir_path)
                .map_err(RuntimeError::from)?
                .map_err(|_| invalid_constant_tag_error())?
                .ok_or_else(|| RuntimeError::new("invalid cached IR"))?;
            &cached
        };
        let _ = ir.write_debug_to(output_path);
        Ok(())
    }

    pub fn run_repl(&mut self, source: &str) -> Result<String, RuntimeError> {
        self.current_source = "<stdin>".to_owned();
        self.current_source_bytes = b"<stdin>".to_vec();
        self.call_frames.clear();
        let result = (|| {
            let mut parser = Parser::new(source, "<stdin>")?;
            for operator in &self.prefix_operators {
                parser.add_prefix_operator(operator.clone());
            }
            for (operator, precedence) in &self.infix_operators {
                parser.add_infix_operator(operator.clone(), *precedence);
            }
            let Some(mut statement) = parser.parse_next()? else {
                return Ok(std::mem::take(&mut self.output));
            };
            if let Stmt::Expression(expression) = statement {
                statement = Stmt::Expression(Expr::Call {
                    callee: Box::new(Expr::Identifier(
                        "println".to_owned(),
                        Location { line: 0, column: 0 },
                    )),
                    arguments: vec![Argument {
                        value: expression,
                        unpack: false,
                    }],
                    location: Location { line: 0, column: 0 },
                });
            }
            let code = self.repl_code.clone();
            let start = code.borrow().instructions().len();
            if let Some(live_context) = &self.repl_live_context {
                live_context.borrow_mut().pc = start;
            }
            code.borrow_mut().add_statement(&statement);
            let environment = self.globals.clone();
            let outcome = if let Some(live_context) = self.repl_live_context.clone() {
                self.resume_live_vm_context(live_context)?
            } else {
                populate_constants(&self.current_constants, &code);
                let context = self.root_vm_context(code, &environment, start);
                let live_context = Rc::new(RefCell::new(context.clone()));
                self.repl_live_context = Some(live_context.clone());
                self.run_vm_context_with_live(context, live_context)?
            };
            self.repl_live_context = Some(outcome.live_context.clone());
            self.globals = outcome.environment;
            Ok(std::mem::take(&mut self.output))
        })();
        let result = result.map_err(|error| self.attach_error(error));
        if result.is_ok() {
            self.call_frames.clear();
        }
        result
    }

    /// Reports whether the legacy REPL parser would execute the first
    /// statement now instead of asking its resume callback for another line.
    pub fn repl_input_complete(&self, source: &str) -> bool {
        let mut parser = match Parser::new(source, "<stdin>") {
            Ok(parser) => parser,
            Err(_) => return true,
        };
        for operator in &self.prefix_operators {
            parser.add_prefix_operator(operator.clone());
        }
        for (operator, precedence) in &self.infix_operators {
            parser.add_infix_operator(operator.clone(), *precedence);
        }
        if parser.has_lex_error() {
            return true;
        }
        let parsed = parser.parse_next();
        if parsed.is_err() {
            return !parser.is_at_end();
        }
        let Some(statement) = parsed.expect("checked above") else {
            return true;
        };
        if !parser.is_at_end() || source.trim_end().ends_with(';') {
            return true;
        }
        match statement {
            Stmt::Expression(_) | Stmt::DoWhile { .. } | Stmt::Return(_) => false,
            Stmt::If {
                ref else_branch, ..
            } => repl_if_has_final_else(else_branch.as_deref()),
            Stmt::Declaration(ref declaration) => {
                declaration.values.is_empty()
                    || repl_block_declaration_is_complete(source, declaration)
            }
            Stmt::Block(_)
            | Stmt::Import { .. }
            | Stmt::PrefixOperator(_)
            | Stmt::InfixOperator { .. }
            | Stmt::While { .. }
            | Stmt::Foreach { .. }
            | Stmt::Break
            | Stmt::Continue => true,
        }
    }

    #[must_use]
    pub fn is_halted(&self) -> bool {
        self.halted
    }

    fn run_internal(
        &mut self,
        source: &[u8],
        name: &str,
        source_name_bytes: Option<Vec<u8>>,
        diagnostic_name_bytes: Option<Vec<u8>>,
        print_expressions: bool,
        ir_path: Option<&Path>,
    ) -> Result<String, RuntimeError> {
        let path = Path::new(name);
        let source_name = path
            .file_name()
            .and_then(|name| name.to_str())
            .unwrap_or(name)
            .to_owned();
        let source_name_bytes =
            source_name_bytes.unwrap_or_else(|| source_name.as_bytes().to_vec());
        let previous_source = std::mem::replace(&mut self.current_source, source_name);
        let previous_source_bytes =
            std::mem::replace(&mut self.current_source_bytes, source_name_bytes);
        let previous_constants = (!print_expressions).then(|| {
            std::mem::replace(
                &mut self.current_constants,
                Rc::new(RefCell::new(HashMap::new())),
            )
        });
        self.call_frames.clear();
        let pushed_directory = path
            .extension()
            .is_some_and(|extension| extension == "anole");
        if pushed_directory {
            let directory = path
                .parent()
                .filter(|parent| !parent.as_os_str().is_empty())
                .unwrap_or_else(|| Path::new("."));
            self.directories.push(directory.to_path_buf());
        }
        let environment = Rc::clone(&self.globals);
        let result = (|| {
            if let Some(ir_path) = ir_path
                && let Some(ir) = fresh_legacy_ir(path, ir_path)?
                && self.supports_cached_ir(&ir)
            {
                let outcome = self.execute_ir_vm(&ir, &environment)?;
                self.globals = outcome.environment;
                self.last_file_ir = Some(ir);
                return Ok(std::mem::take(&mut self.output));
            }
            let outcome =
                self.execute_incremental_ir(source, name, print_expressions, environment.clone())?;
            self.globals = outcome.environment;
            self.last_file_ir = Some(outcome.ir.clone());
            if !self.halted
                && let Some(path) = ir_path
            {
                let _ = outcome.ir.write_to(path);
            }
            Ok(std::mem::take(&mut self.output))
        })();
        let result = result.map_err(|error: RuntimeError| {
            let error = if let Some(raw) = diagnostic_name_bytes.as_deref() {
                error.replace_diagnostic_source(name, raw)
            } else {
                error
            };
            self.attach_error(error)
        });
        if pushed_directory {
            self.directories.pop();
        }
        self.current_source = previous_source;
        self.current_source_bytes = previous_source_bytes;
        if let Some(previous_constants) = previous_constants {
            self.current_constants = previous_constants;
        }
        if result.is_ok() {
            self.call_frames.clear();
        }
        result
    }

    fn supports_cached_ir(&self, ir: &LegacyIr) -> bool {
        ir.instructions()
            .iter()
            .all(|instruction| instruction.opcode != IrOpcode::PlaceHolder)
    }

    fn compile_ir(
        &self,
        source: &[u8],
        name: &str,
        print_expressions: bool,
    ) -> Result<LegacyIr, ParseError> {
        let mut parser = Parser::new_bytes(source, name)?;
        for operator in &self.prefix_operators {
            parser.add_prefix_operator(operator.clone());
        }
        for (operator, precedence) in &self.infix_operators {
            parser.add_infix_operator(operator.clone(), *precedence);
        }
        let mut ir = LegacyIr::default();
        while let Some(mut statement) = parser.parse_next()? {
            if matches!(statement, Stmt::Import { .. }) {
                return Err(Parser::plain_error(
                    Location { line: 0, column: 0 },
                    "imports require incremental execution",
                ));
            }
            if print_expressions && let Stmt::Expression(expression) = statement {
                statement = Stmt::Expression(Expr::Call {
                    callee: Box::new(Expr::Identifier(
                        "println".to_owned(),
                        Location { line: 0, column: 0 },
                    )),
                    arguments: vec![Argument {
                        value: expression,
                        unpack: false,
                    }],
                    location: Location { line: 0, column: 0 },
                });
            }
            ir.add_statement(&statement);
        }
        Ok(ir)
    }

    fn execute_incremental_ir(
        &mut self,
        source: &[u8],
        name: &str,
        print_expressions: bool,
        mut environment: Environment,
    ) -> Result<IncrementalVmOutcome, RuntimeError> {
        let mut parser = Parser::new_bytes(source, name)?;
        let code = Rc::new(RefCell::new(LegacyIr::default()));
        let mut live_context = None;
        let mut root_return = None;
        let mut parser_infix_count = 0;
        loop {
            for operator in &self.prefix_operators {
                parser.add_prefix_operator(operator.clone());
            }
            for (operator, precedence) in self.infix_operators.iter().skip(parser_infix_count) {
                parser.add_infix_operator(operator.clone(), *precedence);
            }
            parser_infix_count = self.infix_operators.len();
            let Some(mut statement) = parser.parse_next()? else {
                break;
            };
            if print_expressions && let Stmt::Expression(expression) = statement {
                statement = Stmt::Expression(Expr::Call {
                    callee: Box::new(Expr::Identifier(
                        "println".to_owned(),
                        Location { line: 0, column: 0 },
                    )),
                    arguments: vec![Argument {
                        value: expression,
                        unpack: false,
                    }],
                    location: Location { line: 0, column: 0 },
                });
            }
            let start = code.borrow().instructions().len();
            code.borrow_mut().add_statement(&statement);
            if root_return.is_some() {
                continue;
            }
            let outcome = if let Some(live_context) = live_context.clone() {
                self.resume_live_vm_context(live_context)?
            } else {
                self.execute_shared_ir_vm(code.clone(), &environment, start)?
            };
            live_context = Some(outcome.live_context.clone());
            environment = outcome.environment;
            if outcome.root_returned {
                root_return = Some(outcome.value);
            }
            if self.halted {
                break;
            }
        }
        let ir = code.borrow().clone();
        Ok(IncrementalVmOutcome {
            ir,
            environment,
            root_return,
        })
    }

    fn execute_ir_vm(
        &mut self,
        ir: &LegacyIr,
        environment: &Environment,
    ) -> Result<VmOutcome, RuntimeError> {
        self.execute_ir_vm_from(ir, environment, 0)
    }

    fn execute_ir_vm_from(
        &mut self,
        ir: &LegacyIr,
        environment: &Environment,
        pc: usize,
    ) -> Result<VmOutcome, RuntimeError> {
        self.execute_shared_ir_vm(Rc::new(RefCell::new(ir.clone())), environment, pc)
    }

    fn execute_shared_ir_vm(
        &mut self,
        code: Rc<RefCell<LegacyIr>>,
        environment: &Environment,
        pc: usize,
    ) -> Result<VmOutcome, RuntimeError> {
        populate_constants(&self.current_constants, &code);
        let context = self.root_vm_context(code, environment, pc);
        self.run_vm_context(context)
    }

    fn root_vm_context(
        &self,
        code: Rc<RefCell<LegacyIr>>,
        environment: &Environment,
        pc: usize,
    ) -> VmContext {
        VmContext {
            pre_context: None,
            live_context: Weak::new(),
            code,
            pc,
            environment: environment.clone(),
            stack: Rc::new(RefCell::new(Vec::new())),
            call_anchors: Vec::new(),
            source: self.current_source.clone(),
            source_bytes: self.current_source_bytes.clone(),
            constants: self.current_constants.clone(),
            trace: Vec::new(),
            kind: VmContextKind::Root,
        }
    }

    fn resume_live_vm_context(
        &mut self,
        live_context: Rc<RefCell<VmContext>>,
    ) -> Result<VmOutcome, RuntimeError> {
        let context = live_context.borrow().clone();
        populate_constants(&context.constants, &context.code);
        self.run_vm_context_with_live(context, live_context)
    }

    fn capture_vm_context(context: &VmContext) -> VmContext {
        let mut captured = context.clone();
        captured.live_context = Weak::new();
        captured.stack = Rc::new(RefCell::new(context.stack.borrow().clone()));
        captured
    }

    fn resumed_vm_context(captured: &VmContext, value: Cell) -> VmContext {
        let mut resumed = captured.clone();
        resumed.live_context = Weak::new();
        resumed.environment = Env::child(&captured.environment);
        resumed.stack = Rc::new(RefCell::new(captured.stack.borrow().clone()));
        resumed.stack.borrow_mut().push(copy_variable(value));
        resumed.pc += 1;
        resumed
    }

    fn run_vm_context(&mut self, context: VmContext) -> Result<VmOutcome, RuntimeError> {
        let live_context = Rc::new(RefCell::new(context.clone()));
        self.run_vm_context_with_live(context, live_context)
    }

    fn run_vm_context_with_live(
        &mut self,
        mut context: VmContext,
        mut live_context: Rc<RefCell<VmContext>>,
    ) -> Result<VmOutcome, RuntimeError> {
        context.live_context = Rc::downgrade(&live_context);
        loop {
            if let Some(context_link) = context.live_context.upgrade() {
                live_context = context_link;
            } else {
                live_context = Rc::new(RefCell::new(context.clone()));
                context.live_context = Rc::downgrade(&live_context);
            }
            *live_context.borrow_mut() = context.clone();
            self.current_source = context.source.clone();
            self.current_source_bytes = context.source_bytes.clone();
            self.current_constants = context.constants.clone();
            self.call_frames = context.trace.clone();
            let (instruction, location) = {
                let code = context.code.borrow();
                (
                    code.instructions().get(context.pc).cloned(),
                    code.location(context.pc),
                )
            };
            let Some(instruction) = instruction else {
                if let Some(pre_context) = context.pre_context.clone() {
                    let mut parent = pre_context.borrow().clone();
                    parent.stack.borrow_mut().push(cell(Value::None));
                    parent.pc += 1;
                    *pre_context.borrow_mut() = parent.clone();
                    context = parent;
                    continue;
                }
                return Ok(VmOutcome {
                    value: cell(Value::None),
                    environment: context.environment,
                    live_context: live_context.clone(),
                    root_returned: false,
                });
            };
            match (instruction.opcode, instruction.operand) {
                (IrOpcode::Pop, IrOperand::Size(size)) => {
                    let size = usize::try_from(size)
                        .map_err(|_| RuntimeError::new("invalid cached IR pop count"))?;
                    let mut stack = context.stack.borrow_mut();
                    let keep = stack
                        .len()
                        .checked_sub(size)
                        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))?;
                    stack.truncate(keep);
                }
                (IrOpcode::Import, IrOperand::String(name)) => {
                    let loaded =
                        self.load_module(&[ModulePart::Name(name)], &context.environment)?;
                    if let Some(value) = loaded.root_return {
                        context.stack.borrow_mut().push(value);
                        context.pc += 1;
                        let importer_live = context
                            .live_context
                            .upgrade()
                            .ok_or_else(|| RuntimeError::new("invalid VM importer context"))?;
                        let outcome = self.run_vm_context_with_live(context, importer_live)?;
                        context = outcome.live_context.borrow().clone();
                        if let Some(cache_key) = loaded.deferred_cache_key {
                            self.modules.insert(cache_key, loaded.value.clone());
                        }
                        context.stack.borrow_mut().push(loaded.value);
                        context.pc += 1;
                        continue;
                    }
                    context.stack.borrow_mut().push(loaded.value);
                }
                (IrOpcode::ImportPath, IrOperand::Bytes(path)) => {
                    let loaded =
                        self.load_module(&[ModulePart::Path(path)], &context.environment)?;
                    if let Some(value) = loaded.root_return {
                        context.stack.borrow_mut().push(value);
                        context.pc += 1;
                        let importer_live = context
                            .live_context
                            .upgrade()
                            .ok_or_else(|| RuntimeError::new("invalid VM importer context"))?;
                        let outcome = self.run_vm_context_with_live(context, importer_live)?;
                        context = outcome.live_context.borrow().clone();
                        if let Some(cache_key) = loaded.deferred_cache_key {
                            self.modules.insert(cache_key, loaded.value.clone());
                        }
                        context.stack.borrow_mut().push(loaded.value);
                        context.pc += 1;
                        continue;
                    }
                    context.stack.borrow_mut().push(loaded.value);
                }
                (IrOpcode::ImportAll, IrOperand::None) => {
                    let module = vm_pop(&context)?;
                    match &**module.borrow() {
                        Value::Namespace(namespace) => {
                            for (name, value) in namespace.borrow().values.clone() {
                                Env::define(&context.environment, name, value);
                            }
                        }
                        _ => {
                            return Err(RuntimeError::new(format!(
                                "{} is not a module",
                                previous_import_name(&context)
                            )));
                        }
                    }
                }
                (IrOpcode::ImportPart, IrOperand::String(name)) => {
                    let module = context
                        .stack
                        .borrow()
                        .last()
                        .cloned()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))?;
                    if !matches!(&**module.borrow(), Value::Namespace(_)) {
                        return Err(RuntimeError::new(format!(
                            "{} is not a module",
                            previous_import_name(&context)
                        )));
                    }
                    let member = self.member(module, &name, location).map_err(|mut error| {
                        if context.code.borrow().mapped_location(context.pc).is_none() {
                            error.location = None;
                        }
                        error
                    })?;
                    context.stack.borrow_mut().push(member);
                }
                (IrOpcode::Load, IrOperand::String(name)) => {
                    let value = Env::load(&context.environment, &name);
                    let thunk = match &**value.borrow() {
                        Value::IrThunk(thunk) => Some(thunk.clone()),
                        _ => None,
                    };
                    if let Some(thunk) = thunk {
                        if let Some(cached) = thunk.cached.borrow().clone() {
                            context.stack.borrow_mut().push(cached);
                            context.pc += 1;
                            continue;
                        }
                        let stack = context.stack.clone();
                        let trace = context.trace.clone();
                        let pre_context = live_context.clone();
                        context = VmContext {
                            pre_context: Some(pre_context),
                            live_context: Weak::new(),
                            code: Rc::new(RefCell::new((*thunk.code).clone())),
                            pc: thunk.base,
                            environment: Env::child(&thunk.environment),
                            stack,
                            call_anchors: Vec::new(),
                            source: thunk.source.clone(),
                            source_bytes: thunk.source_bytes.clone(),
                            constants: thunk.constants.clone(),
                            trace,
                            kind: VmContextKind::Thunk(thunk),
                        };
                        continue;
                    }
                    context.stack.borrow_mut().push(value);
                }
                (IrOpcode::LoadConst, IrOperand::Size(index)) => {
                    let value = self.cached_constant(&context.code.borrow(), index)?;
                    context.stack.borrow_mut().push(value);
                }
                (IrOpcode::LoadMember, IrOperand::String(name)) => {
                    let receiver = vm_pop(&context)?;
                    let member = self.member(receiver, &name, location)?;
                    context.stack.borrow_mut().push(member);
                }
                (IrOpcode::Store, IrOperand::None) => {
                    let target = vm_pop(&context)?;
                    let value = vm_pop(&context)?;
                    ensure_bound(&value)?;
                    *target.borrow_mut() = value.borrow().clone();
                    context.stack.borrow_mut().push(target);
                }
                (IrOpcode::StoreRef, IrOperand::String(name)) => {
                    self.bind(
                        &Binding::Name {
                            name,
                            by_reference: true,
                        },
                        vm_pop(&context)?,
                        &context.environment,
                    )?;
                }
                (IrOpcode::StoreLocal, IrOperand::String(name)) => {
                    self.bind(
                        &Binding::Name {
                            name,
                            by_reference: false,
                        },
                        vm_pop(&context)?,
                        &context.environment,
                    )?;
                }
                (IrOpcode::NewScope, IrOperand::None) => {
                    context.environment = Env::child(&context.environment);
                }
                (IrOpcode::EndScope, IrOperand::None) => {
                    let parent = context
                        .environment
                        .borrow()
                        .parent
                        .clone()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR scope"))?;
                    context.environment = parent;
                }
                (IrOpcode::CallAc, IrOperand::None) => {
                    context.call_anchors.push(context.stack.borrow().len());
                }
                (IrOpcode::Call, IrOperand::None) => {
                    let anchor = context
                        .call_anchors
                        .pop()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR call anchor"))?;
                    let count = context
                        .stack
                        .borrow()
                        .len()
                        .checked_sub(anchor + 1)
                        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))?;
                    context = self.vm_call(context, count, location)?;
                    continue;
                }
                (IrOpcode::FastCall, IrOperand::Size(size)) => {
                    let size = usize::try_from(size)
                        .map_err(|_| RuntimeError::new("invalid cached IR argument count"))?;
                    context = self.vm_call(context, size, location)?;
                    continue;
                }
                (IrOpcode::Return, IrOperand::None) => {
                    let value = vm_pop(&context)?;
                    let Some(pre_context) = context.pre_context.clone() else {
                        return Ok(VmOutcome {
                            value,
                            environment: context.environment,
                            live_context: live_context.clone(),
                            root_returned: true,
                        });
                    };
                    let mut parent = pre_context.borrow().clone();
                    parent.stack.borrow_mut().push(value);
                    parent.pc += 1;
                    *pre_context.borrow_mut() = parent.clone();
                    context = parent;
                    continue;
                }
                (IrOpcode::ReturnNone, IrOperand::None) => {
                    let value = cell(Value::None);
                    let Some(pre_context) = context.pre_context.clone() else {
                        return Ok(VmOutcome {
                            value,
                            environment: context.environment,
                            live_context: live_context.clone(),
                            root_returned: true,
                        });
                    };
                    let mut parent = pre_context.borrow().clone();
                    parent.stack.borrow_mut().push(value);
                    parent.pc += 1;
                    *pre_context.borrow_mut() = parent.clone();
                    context = parent;
                    continue;
                }
                (IrOpcode::Jump, IrOperand::Size(target)) => {
                    context.pc = ir_target(target)?;
                    continue;
                }
                (IrOpcode::JumpIf, IrOperand::Size(target)) => {
                    if self.truthy(&vm_pop(&context)?, location)? {
                        context.pc = ir_target(target)?;
                        continue;
                    }
                }
                (IrOpcode::JumpIfNot, IrOperand::Size(target)) => {
                    if !self.truthy(&vm_pop(&context)?, location)? {
                        context.pc = ir_target(target)?;
                        continue;
                    }
                }
                (IrOpcode::Match, IrOperand::Size(target)) => {
                    let key = vm_pop(&context)?;
                    let value = context
                        .stack
                        .borrow()
                        .last()
                        .cloned()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))?;
                    if self.match_equal(value, key, location, &context.environment)? {
                        context.stack.borrow_mut().pop();
                        context.pc = ir_target(target)?;
                        continue;
                    }
                }
                (IrOpcode::AddPrefixOp, IrOperand::String(operator)) => {
                    self.prefix_operators.insert(operator);
                }
                (IrOpcode::AddInfixOp, IrOperand::StringSize(operator, precedence)) => {
                    self.infix_operators.push((operator, precedence));
                }
                (IrOpcode::Pack, IrOperand::None) => {
                    context
                        .stack
                        .borrow_mut()
                        .push(cell(list_value(Vec::new())));
                }
                (IrOpcode::Unpack, IrOperand::Size(expected)) => {
                    let value = vm_pop(&context)?;
                    ensure_bound(&value)?;
                    let values: Vec<Cell> = match &**value.borrow() {
                        Value::List(values) => values
                            .borrow()
                            .iter()
                            .map(|entry| entry.value.clone())
                            .collect(),
                        _ => return Err(RuntimeError::new("expect list expr")),
                    };
                    if expected != 0 && values.len() as u64 != expected {
                        return Err(RuntimeError::new(format!(
                            "expect {expected} but given {}",
                            values.len()
                        )));
                    }
                    context.stack.borrow_mut().extend(values.into_iter().rev());
                }
                (IrOpcode::LambdaDecl, IrOperand::SizePair(parameters, target)) => {
                    let parameter_count = usize::try_from(parameters)
                        .map_err(|_| RuntimeError::new("invalid cached IR parameter count"))?;
                    let function = cell(Value::IrFunction(Rc::new(IrFunction {
                        code: Rc::new(context.code.borrow().clone()),
                        base: context.pc + 1,
                        parameter_count,
                        closure: Env::child(&context.environment),
                        source: context.source.clone(),
                        source_bytes: context.source_bytes.clone(),
                        constants: context.constants.clone(),
                    })));
                    context.stack.borrow_mut().push(function);
                    context.pc = ir_target(target)?;
                    continue;
                }
                (IrOpcode::ThunkDecl, IrOperand::Size(target)) => {
                    let thunk = cell(Value::IrThunk(IrThunk {
                        code: Rc::new(context.code.borrow().clone()),
                        base: context.pc + 1,
                        environment: Env::child(&context.environment),
                        cached: Rc::new(RefCell::new(None)),
                        source: context.source.clone(),
                        source_bytes: context.source_bytes.clone(),
                        constants: context.constants.clone(),
                    }));
                    context.stack.borrow_mut().push(thunk);
                    context.pc = ir_target(target)?;
                    continue;
                }
                (IrOpcode::ThunkOver, IrOperand::None) => {
                    let result = vm_pop(&context)?;
                    let VmContextKind::Thunk(thunk) = &context.kind else {
                        return Err(RuntimeError::new("invalid cached IR thunk return"));
                    };
                    *thunk.cached.borrow_mut() = Some(result.clone());
                    let Some(pre_context) = context.pre_context.clone() else {
                        return Ok(VmOutcome {
                            value: result,
                            environment: context.environment,
                            live_context: live_context.clone(),
                            root_returned: false,
                        });
                    };
                    let mut parent = pre_context.borrow().clone();
                    parent.stack.borrow_mut().push(result);
                    parent.pc += 1;
                    *pre_context.borrow_mut() = parent.clone();
                    context = parent;
                    continue;
                }
                (IrOpcode::Neg, IrOperand::None) => {
                    let value = vm_pop(&context)?;
                    let result = self.unary("-", value, location, &context.environment)?;
                    context.stack.borrow_mut().push(result);
                }
                (IrOpcode::BNeg, IrOperand::None) => {
                    let value = vm_pop(&context)?;
                    let result = self.unary("~", value, location, &context.environment)?;
                    context.stack.borrow_mut().push(result);
                }
                (opcode, IrOperand::None)
                    if matches!(
                        opcode,
                        IrOpcode::Add
                            | IrOpcode::Sub
                            | IrOpcode::Mul
                            | IrOpcode::Div
                            | IrOpcode::Mod
                            | IrOpcode::Is
                            | IrOpcode::Ceq
                            | IrOpcode::Cne
                            | IrOpcode::Clt
                            | IrOpcode::Cle
                            | IrOpcode::BOr
                            | IrOpcode::BXor
                            | IrOpcode::BAnd
                            | IrOpcode::Bls
                            | IrOpcode::Brs
                    ) =>
                {
                    let right = vm_pop(&context)?;
                    let left = vm_pop(&context)?;
                    let operator = match opcode {
                        IrOpcode::Add => "+",
                        IrOpcode::Sub => "-",
                        IrOpcode::Mul => "*",
                        IrOpcode::Div => "/",
                        IrOpcode::Mod => "%",
                        IrOpcode::Is => "is",
                        IrOpcode::Ceq => "=",
                        IrOpcode::Cne => "!=",
                        IrOpcode::Clt => "<",
                        IrOpcode::Cle => "<=",
                        IrOpcode::BOr => "|",
                        IrOpcode::BXor => "^",
                        IrOpcode::BAnd => "&",
                        IrOpcode::Bls => "<<",
                        IrOpcode::Brs => ">>",
                        _ => unreachable!(),
                    };
                    let result =
                        self.binary(operator, left, right, location, &context.environment)?;
                    context.stack.borrow_mut().push(result);
                }
                (IrOpcode::Index, IrOperand::None) => {
                    let object = vm_pop(&context)?;
                    let index = vm_pop(&context)?;
                    let result = self.index_values(object, index, location)?;
                    context.stack.borrow_mut().push(result);
                }
                (IrOpcode::BuildEnum, IrOperand::None) => {
                    let values = context.environment.borrow().values.clone();
                    let namespace = Env::root();
                    namespace.borrow_mut().values = values;
                    let parent = context
                        .environment
                        .borrow()
                        .parent
                        .clone()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR enum scope"))?;
                    context.environment = parent;
                    context
                        .stack
                        .borrow_mut()
                        .push(cell(Value::Enum(namespace)));
                }
                (IrOpcode::BuildList, IrOperand::Size(size)) => {
                    let size = usize::try_from(size)
                        .map_err(|_| RuntimeError::new("invalid cached IR list size"))?;
                    let mut values = Vec::with_capacity(size);
                    for _ in 0..size {
                        let value = vm_pop(&context)?;
                        ensure_bound(&value)?;
                        values.push(copy_variable(value));
                    }
                    context.stack.borrow_mut().push(cell(list_value(values)));
                }
                (IrOpcode::BuildDict, IrOperand::Size(size)) => {
                    let size = usize::try_from(size)
                        .map_err(|_| RuntimeError::new("invalid cached IR dict size"))?;
                    let mut values = Vec::with_capacity(size);
                    for _ in 0..size {
                        let key = vm_pop(&context)?;
                        ensure_bound(&key)?;
                        let value = vm_pop(&context)?;
                        ensure_bound(&value)?;
                        let key = copy_variable(key);
                        let value = copy_variable(value);
                        dict_insert(&mut values, key, value)?;
                    }
                    context.stack.borrow_mut().push(cell(dict_value(values)));
                }
                (IrOpcode::BuildClass, IrOperand::String(_)) => {
                    let anchor = context
                        .call_anchors
                        .pop()
                        .ok_or_else(|| RuntimeError::new("invalid cached IR call anchor"))?;
                    let count = context
                        .stack
                        .borrow()
                        .len()
                        .checked_sub(anchor)
                        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))?;
                    let mut bases = Vec::with_capacity(count);
                    for _ in 0..count {
                        let base = vm_pop(&context)?;
                        ensure_bound(&base)?;
                        let Value::Class(class) = &**base.borrow() else {
                            return Err(RuntimeError::new(
                                "each base of one class must be one class",
                            ));
                        };
                        bases.push(class.clone());
                    }
                    let members = Env::child(&context.environment);
                    let constructors = inherit_class_members(&members, &bases);
                    Env::define(
                        &members,
                        "bctors".to_owned(),
                        cell(list_value(constructors)),
                    );
                    context
                        .stack
                        .borrow_mut()
                        .push(cell(Value::Class(Rc::new(Class {
                            members: members.clone(),
                        }))));
                    context.environment = members;
                }
                _ => return Err(RuntimeError::new("unsupported cached IR instruction")),
            }
            if self.halted {
                return Ok(VmOutcome {
                    value: cell(Value::None),
                    environment: context.environment,
                    live_context: live_context.clone(),
                    root_returned: false,
                });
            }
            context.pc += 1;
        }
    }

    fn vm_call(
        &mut self,
        mut context: VmContext,
        size: usize,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        let callee = vm_pop(&context)?;
        if let Some(live_context) = context.live_context.upgrade() {
            *live_context.borrow_mut() = context.clone();
        }
        ensure_bound(&callee).map_err(|error| error_at_location(error, location))?;
        let value = callee.borrow().value.clone();
        match value {
            Value::IrFunction(function) => {
                let arguments = Self::vm_pop_arguments(&context, size)?;
                self.ir_function_vm_context(context, function, arguments, location)
            }
            Value::UserIrMethod { function, receiver } => {
                let mut arguments = Self::vm_pop_arguments(&context, size)?;
                arguments.insert(0, receiver);
                self.ir_function_vm_context(context, function, arguments, location)
            }
            Value::Continuation(captured) => {
                if size != 1 {
                    return Err(RuntimeError::at(location, "continuation need a argument"));
                }
                let value = vm_pop(&context)?;
                ensure_bound(&value).map_err(|error| error_at_location(error, location))?;
                Ok(Self::resumed_vm_context(&captured, value))
            }
            Value::Builtin(Builtin::CallWithCurrentContinuation) => {
                // This builtin pops only the callable at the top of the shared
                // operand stack. Extra arguments remain in the captured context.
                let function = vm_pop(&context)
                    .map_err(|_| RuntimeError::at(location, "call/cc expects one argument"))?;
                ensure_bound(&function).map_err(|error| error_at_location(error, location))?;
                if !matches!(
                    &**function.borrow(),
                    Value::IrFunction(_) | Value::Continuation(_)
                ) {
                    return Err(RuntimeError::at(
                        location,
                        "err type as the argument for call/cc",
                    ));
                }
                let captured = Rc::new(Self::capture_vm_context(&context));
                let continuation = cell(Value::Continuation(captured));
                let function_value = function.borrow().value.clone();
                match function_value {
                    Value::IrFunction(function) => self.callcc_ir_function_vm_context(
                        context,
                        function,
                        continuation,
                        location,
                    ),
                    Value::Continuation(_) => {
                        self.vm_call_value(context, function, vec![continuation], location)
                    }
                    _ => unreachable!(),
                }
            }
            Value::Builtin(Builtin::Eval) => {
                let arguments = Self::vm_pop_arguments(&context, 1)?;
                self.eval_vm_context(context, arguments, location)
            }
            Value::Builtin(builtin) => self.vm_call_builtin(context, builtin, size, location),
            Value::BoundMethod { receiver, name } => {
                let consumed = native_method_consumed_arguments(&receiver, &name);
                let arguments = Self::vm_pop_arguments(&context, consumed)?;
                let pushes_result = native_method_pushes_result(
                    &receiver,
                    &name,
                    context.stack.borrow().is_empty(),
                );
                let result = self
                    .call_method(receiver, &name, arguments)
                    .map_err(|error| error_at_location(error, location))?;
                if pushes_result {
                    context.stack.borrow_mut().push(result);
                }
                context.pc += 1;
                Ok(context)
            }
            Value::Class(class) => {
                let mut arguments = Self::vm_pop_arguments(&context, size)?;
                let instance = cell(Value::Instance {
                    class: class.clone(),
                    fields: clone_class_members(&class),
                });
                if let Some(constructor) = class_member(&class, "__init__") {
                    let constructor = constructor.borrow().value.clone();
                    arguments.insert(0, instance);
                    match constructor {
                        Value::IrFunction(function) => {
                            self.ir_function_vm_context(context, function, arguments, location)
                        }
                        _ => Err(RuntimeError::at(location, "__init__ must be callable")),
                    }
                } else if arguments.is_empty() {
                    // A default construction preserves an existing operand as
                    // the observable result, with the instance as an empty-stack
                    // fallback.
                    if context.stack.borrow().is_empty() {
                        context.stack.borrow_mut().push(instance);
                    }
                    context.pc += 1;
                    Ok(context)
                } else {
                    Err(RuntimeError::at(
                        location,
                        "only default ctor but given non-zero arguments",
                    ))
                }
            }
            _ => Err(RuntimeError::at(
                location,
                "failed call with the given non-function",
            )),
        }
    }

    fn vm_pop_arguments(context: &VmContext, count: usize) -> Result<Vec<Cell>, RuntimeError> {
        let mut arguments = Vec::with_capacity(count);
        for _ in 0..count {
            arguments.push(vm_pop(context)?);
        }
        Ok(arguments)
    }

    fn vm_call_builtin(
        &mut self,
        mut context: VmContext,
        builtin: Builtin,
        size: usize,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        if matches!(builtin, Builtin::Print | Builtin::Println) {
            let value = context
                .stack
                .borrow()
                .last()
                .cloned()
                .ok_or_else(|| RuntimeError::at(location, "invalid cached IR stack"))?;
            ensure_bound(&value).map_err(|error| error_at_location(error, location))?;
            if !matches!(**value.borrow(), Value::None) {
                vm_pop(&context)?;
                let rendered = self
                    .render_bytes(&value)
                    .map_err(|error| error_at_location(error, location))?;
                self.emit_bytes(&rendered)
                    .map_err(|error| error_at_location(error, location))?;
                if matches!(builtin, Builtin::Println) {
                    self.emit_bytes(b"\n")
                        .map_err(|error| error_at_location(error, location))?;
                }
            }
            context.stack.borrow_mut().push(cell(Value::None));
            context.pc += 1;
            return Ok(context);
        }

        let consumed = match builtin {
            Builtin::Str | Builtin::Type | Builtin::Id => 1,
            Builtin::Time | Builtin::Input | Builtin::Exit => 0,
            Builtin::Args | Builtin::CurrentPath => {
                if size != 0 {
                    let message = if matches!(builtin, Builtin::Args) {
                        "args need no arguments"
                    } else {
                        "function current_path need no arguments"
                    };
                    return Err(RuntimeError::at(location, message));
                }
                0
            }
            Builtin::Open => {
                if size != 2 {
                    return Err(RuntimeError::at(location, "function open need 2 arguments"));
                }
                2
            }
            Builtin::IsDirectory => {
                if size != 1 {
                    return Err(RuntimeError::at(
                        location,
                        "function current_path need 1 argument",
                    ));
                }
                1
            }
            Builtin::ReadDirectory => {
                if size != 1 {
                    return Err(RuntimeError::at(
                        location,
                        "function read_dir need only one argument",
                    ));
                }
                1
            }
            Builtin::Print
            | Builtin::Println
            | Builtin::Eval
            | Builtin::CallWithCurrentContinuation => unreachable!(),
        };
        let arguments = Self::vm_pop_arguments(&context, consumed)?;
        let result = self
            .call_builtin(builtin, arguments, &context.environment)
            .map_err(|error| error_at_location(error, location))?;
        context.stack.borrow_mut().push(result);
        context.pc += 1;
        Ok(context)
    }

    fn vm_call_value(
        &mut self,
        context: VmContext,
        callee: Cell,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        let argument_count = arguments.len();
        context
            .stack
            .borrow_mut()
            .extend(arguments.into_iter().rev());
        context.stack.borrow_mut().push(callee);
        self.vm_call(context, argument_count, location)
    }

    fn eval_vm_context(
        &mut self,
        parent: VmContext,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        let source = arguments
            .first()
            .cloned()
            .unwrap_or_else(|| cell(Value::None));
        ensure_bound(&source)?;
        let source = match &**source.borrow() {
            Value::String(source) => source.clone(),
            _ => return Err(RuntimeError::new("eval expects a string")),
        };
        let mut eval_source = b"return ".to_vec();
        eval_source.extend_from_slice(&source);
        eval_source.push(b';');
        let mut parser = Parser::new_bytes(&eval_source, "<eval>")?;
        for operator in &self.prefix_operators {
            parser.add_prefix_operator(operator.clone());
        }
        for (operator, precedence) in &self.infix_operators {
            parser.add_infix_operator(operator.clone(), *precedence);
        }
        let mut ir = LegacyIr::default();
        if let Some(statement) = parser.parse_next()? {
            ir.add_statement(&statement);
        }
        let constants = constant_pool(&ir);
        let mut trace = parent.trace.clone();
        trace.push((parent.source_bytes.clone(), location));
        let stack = parent.stack.clone();
        let environment = parent.environment.clone();
        Ok(VmContext {
            pre_context: Some(
                parent
                    .live_context
                    .upgrade()
                    .ok_or_else(|| RuntimeError::new("invalid VM parent context"))?,
            ),
            live_context: Weak::new(),
            code: Rc::new(RefCell::new(ir)),
            pc: 0,
            environment,
            stack,
            call_anchors: Vec::new(),
            source: "<eval>".to_owned(),
            source_bytes: b"<eval>".to_vec(),
            constants,
            trace,
            kind: VmContextKind::Function,
        })
    }

    fn callcc_ir_function_vm_context(
        &mut self,
        parent: VmContext,
        function: Rc<IrFunction>,
        continuation: Cell,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        let instruction = function
            .code
            .instructions()
            .get(function.base)
            .ok_or_else(|| RuntimeError::new("invalid cached IR call/cc function"))?;
        let IrOperand::String(name) = &instruction.operand else {
            return Err(RuntimeError::plain_diagnostic("bad any_cast"));
        };
        let environment = Env::child(&function.closure);
        // call/cc binds the first parameter locally regardless of StoreRef and
        // begins execution immediately after that parameter store.
        self.bind(
            &Binding::Name {
                name: name.clone(),
                by_reference: false,
            },
            continuation,
            &environment,
        )?;
        let mut trace = parent.trace.clone();
        trace.push((parent.source_bytes.clone(), location));
        let stack = parent.stack.clone();
        Ok(VmContext {
            pre_context: Some(
                parent
                    .live_context
                    .upgrade()
                    .ok_or_else(|| RuntimeError::new("invalid VM parent context"))?,
            ),
            live_context: Weak::new(),
            code: Rc::new(RefCell::new((*function.code).clone())),
            pc: function.base + 1,
            environment,
            stack,
            call_anchors: Vec::new(),
            source: function.source.clone(),
            source_bytes: function.source_bytes.clone(),
            constants: function.constants.clone(),
            trace,
            kind: VmContextKind::Function,
        })
    }

    fn ir_function_vm_context(
        &mut self,
        parent: VmContext,
        function: Rc<IrFunction>,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<VmContext, RuntimeError> {
        let environment = Env::child(&function.closure);
        let mut pc = function.base;
        let mut argument_index = 0;
        let mut parameter_count = function.parameter_count;
        while argument_index < arguments.len() && parameter_count > 0 {
            let instruction = function
                .code
                .instructions()
                .get(pc)
                .ok_or_else(|| RuntimeError::new("invalid cached IR function"))?;
            match (&instruction.opcode, &instruction.operand) {
                (IrOpcode::Pack, IrOperand::None) => {
                    let store = function
                        .code
                        .instructions()
                        .get(pc + 1)
                        .ok_or_else(|| RuntimeError::new("invalid cached IR function"))?;
                    let by_reference = store.opcode == IrOpcode::StoreRef;
                    let IrOperand::String(name) = &store.operand else {
                        return Err(RuntimeError::new("invalid cached IR packed parameter"));
                    };
                    let items = arguments[argument_index..]
                        .iter()
                        .map(|argument| {
                            if by_reference {
                                Ok(argument.clone())
                            } else {
                                ensure_bound(argument)?;
                                Ok(copy_variable(argument.clone()))
                            }
                        })
                        .collect::<Result<Vec<_>, RuntimeError>>()?;
                    self.bind(
                        &Binding::Name {
                            name: name.clone(),
                            by_reference: false,
                        },
                        cell(list_value(items)),
                        &environment,
                    )?;
                    argument_index = arguments.len();
                    parameter_count -= 1;
                    pc += 2;
                }
                (IrOpcode::StoreRef, IrOperand::String(name)) => {
                    self.bind(
                        &Binding::Name {
                            name: name.clone(),
                            by_reference: true,
                        },
                        arguments[argument_index].clone(),
                        &environment,
                    )?;
                    argument_index += 1;
                    parameter_count -= 1;
                    pc += 1;
                }
                (IrOpcode::StoreLocal, IrOperand::String(name)) => {
                    self.bind(
                        &Binding::Name {
                            name: name.clone(),
                            by_reference: false,
                        },
                        arguments[argument_index].clone(),
                        &environment,
                    )?;
                    argument_index += 1;
                    parameter_count -= 1;
                    pc += 1;
                }
                (IrOpcode::LambdaDecl, IrOperand::SizePair(_, target))
                | (IrOpcode::ThunkDecl, IrOperand::Size(target)) => {
                    pc = ir_target(*target)?;
                }
                _ => pc += 1,
            }
        }
        if argument_index < arguments.len() {
            return Err(function_call_error(
                &parent,
                &function,
                pc,
                location,
                format!(
                    "function takes {} arguments but {} were given",
                    function.parameter_count,
                    arguments.len()
                )
                .into_bytes(),
            ));
        }
        if parameter_count > 0
            && let Some(instruction) = function.code.instructions().get(pc)
            && matches!(
                instruction.opcode,
                IrOpcode::StoreRef | IrOpcode::StoreLocal
            )
        {
            let IrOperand::String(name) = &instruction.operand else {
                return Err(RuntimeError::new("invalid cached IR parameter"));
            };
            let mut message = b"missing the parameter named '".to_vec();
            message.extend(symbol_to_bytes(name));
            message.push(b'\'');
            return Err(function_call_error(
                &parent, &function, pc, location, message,
            ));
        }
        let mut trace = parent.trace.clone();
        trace.push((parent.source_bytes.clone(), location));
        let stack = parent.stack.clone();
        Ok(VmContext {
            pre_context: Some(
                parent
                    .live_context
                    .upgrade()
                    .ok_or_else(|| RuntimeError::new("invalid VM parent context"))?,
            ),
            live_context: Weak::new(),
            code: Rc::new(RefCell::new((*function.code).clone())),
            pc,
            environment,
            stack,
            call_anchors: Vec::new(),
            source: function.source.clone(),
            source_bytes: function.source_bytes.clone(),
            constants: function.constants.clone(),
            trace,
            kind: VmContextKind::Function,
        })
    }

    fn cached_constant(&self, ir: &LegacyIr, index: u64) -> Result<Cell, RuntimeError> {
        match index {
            0 => Ok(cell(Value::None)),
            1 => Ok(cell(Value::Bool(true))),
            2 => Ok(cell(Value::Bool(false))),
            _ => {
                let offset = usize::try_from(index - 3)
                    .map_err(|_| RuntimeError::new("invalid cached IR constant"))?;
                let (key, _) = ir
                    .constants()
                    .get(offset)
                    .ok_or_else(|| RuntimeError::new("invalid cached IR constant"))?;
                let value = self
                    .current_constants
                    .borrow()
                    .get(key)
                    .cloned()
                    .ok_or_else(|| RuntimeError::new("invalid cached IR constant"))?;
                Ok(copy_variable(value))
            }
        }
    }

    fn index_values(
        &mut self,
        object: Cell,
        index: Cell,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        ensure_bound(&object).map_err(|error| error_at_location(error, location))?;
        ensure_bound(&index).map_err(|error| error_at_location(error, location))?;
        let index_value = index.borrow().value.clone();
        match (&mut **object.borrow_mut(), index_value) {
            (Value::List(items), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                items
                    .borrow()
                    .get(index)
                    .map(|entry| entry.value.clone())
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::String(value), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                value
                    .get(index)
                    .map(|byte| cell(Value::String(vec![*byte])))
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::Dict(values), _) => dict_index(&mut values.borrow_mut(), index),
            (_, Value::Integer(_)) => Err(RuntimeError::at(location, "not support index")),
            _ => Err(RuntimeError::at(location, "index should be integer")),
        }
    }

    fn bind(
        &mut self,
        binding: &Binding,
        value: Cell,
        environment: &Environment,
    ) -> Result<(), RuntimeError> {
        match binding {
            Binding::Name { name, by_reference } => {
                let value = if *by_reference {
                    self.reference(value)?
                } else {
                    ensure_bound(&value)?;
                    copy_variable(value)
                };
                if !*by_reference && let Some(existing) = Env::find_local(environment, name) {
                    *existing.borrow_mut() = value.borrow().clone();
                } else {
                    Env::define(environment, name.clone(), value);
                }
            }
            Binding::Destructure(bindings) => {
                let items: Vec<Cell> = match &**value.borrow() {
                    Value::List(items) => items
                        .borrow()
                        .iter()
                        .map(|entry| entry.value.clone())
                        .collect(),
                    _ => return Err(RuntimeError::new("expect list expr")),
                };
                if items.len() != bindings.len() {
                    return Err(RuntimeError::new(format!(
                        "expect {} but given {}",
                        bindings.len(),
                        items.len()
                    )));
                }
                for (index, binding) in bindings.iter().enumerate() {
                    let item = items[index].clone();
                    self.bind(binding, item, environment)?;
                }
            }
        }
        Ok(())
    }

    fn member(
        &mut self,
        receiver: Cell,
        name: &str,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        ensure_bound(&receiver).map_err(|error| error_at_location(error, location))?;
        let direct_member = match &**receiver.borrow() {
            Value::Namespace(namespace) => Env::find_local(namespace, name),
            Value::Enum(namespace) => Env::find_local(namespace, name).map(copy_variable),
            Value::IrFunction(function) => Some(Env::load(&function.closure, name)),
            Value::Class(class) => {
                if let Some(value) = class_member(class, name) {
                    let member = value.borrow().value.clone();
                    if let Value::IrFunction(function) = member {
                        return Ok(cell(Value::UserIrMethod {
                            function,
                            receiver: copy_variable(receiver.clone()),
                        }));
                    }
                    Some(value)
                } else {
                    None
                }
            }
            Value::Instance { class, fields } => {
                if let Some(value) = Env::find(fields, name) {
                    let member = value.borrow().value.clone();
                    if let Value::IrFunction(function) = member {
                        return Ok(cell(Value::UserIrMethod {
                            function,
                            receiver: copy_variable(receiver.clone()),
                        }));
                    }
                    Some(value)
                } else if let Some(value) = class_member(class, name) {
                    let member = value.borrow().value.clone();
                    if let Value::IrFunction(function) = member {
                        return Ok(cell(Value::UserIrMethod {
                            function,
                            receiver: copy_variable(receiver.clone()),
                        }));
                    }
                    Some(value)
                } else {
                    Some(Env::load(fields, name))
                }
            }
            _ => None,
        };
        if let Some(member) = direct_member {
            name_unbound(&member, name);
            return Ok(member);
        }
        let supported = match &**receiver.borrow() {
            Value::List(_) => matches!(
                name,
                "empty"
                    | "size"
                    | "push"
                    | "pop"
                    | "pop_front"
                    | "front"
                    | "back"
                    | "clear"
                    | "__iterator__"
            ),
            Value::ListIterator(_) => matches!(name, "__has_next__" | "__next__"),
            Value::String(_) => matches!(name, "size" | "to_int"),
            Value::Integer(_) => name == "to_str",
            Value::Dict(_) => {
                matches!(name, "empty" | "size" | "at" | "insert" | "erase" | "clear")
            }
            Value::File(_) => matches!(
                name,
                "good"
                    | "eof"
                    | "close"
                    | "flush"
                    | "read"
                    | "readline"
                    | "write"
                    | "tellg"
                    | "tellp"
                    | "seekg"
                    | "seekp"
            ),
            Value::Path(_) => name == "is_directory",
            _ => false,
        };
        if supported {
            Ok(cell(Value::BoundMethod {
                receiver: copy_variable(receiver),
                name: name.to_owned(),
            }))
        } else if let Value::Dict(values) = &mut **receiver.borrow_mut() {
            dict_index(
                &mut values.borrow_mut(),
                cell(Value::String(symbol_to_bytes(name))),
            )
        } else {
            let mut error = symbol_runtime_error(b"no member named ", name, b"");
            error.location = Some(location);
            Err(error)
        }
    }

    fn object_type_name(&self, value: &Value) -> String {
        match value {
            Value::Unbound(_) => unreachable!("unbound values have no type"),
            Value::None => "none",
            Value::Bool(_) => "boolean",
            Value::Integer(_) => "integer",
            Value::Float(_) => "float",
            Value::String(_) => "string",
            Value::Enum(_) => "enum",
            Value::List(_) => "list",
            Value::ListIterator(_) => "listiterator",
            Value::Dict(_) => "dict",
            Value::Builtin(_) | Value::BoundMethod { .. } => "builtinfunc",
            Value::IrFunction(_) => "func",
            Value::IrThunk(_) => "thunk",
            Value::Continuation(_) => "cont",
            Value::Namespace(_) => "anolemodule",
            Value::Class(_) => "class",
            Value::UserIrMethod { .. } => "method",
            Value::Instance { .. } => "instance",
            Value::File(_) => "file",
            Value::Path(_) => "path",
        }
        .to_owned()
    }

    fn call_builtin(
        &mut self,
        builtin: Builtin,
        arguments: Vec<Cell>,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let first = || {
            arguments
                .first()
                .cloned()
                .unwrap_or_else(|| cell(Value::None))
        };
        match builtin {
            Builtin::Print | Builtin::Println => {
                let value = first();
                ensure_bound(&value)?;
                if arguments.is_empty() && matches!(builtin, Builtin::Println) {
                    self.emit_bytes(b"\n")?;
                } else if !matches!(**value.borrow(), Value::None) {
                    let rendered = self.render_bytes(&value)?;
                    self.emit_bytes(&rendered)?;
                    if matches!(builtin, Builtin::Println) {
                        self.emit_bytes(b"\n")?;
                    }
                }
                Ok(cell(Value::None))
            }
            Builtin::Str => {
                let value = first();
                ensure_bound(&value)?;
                Ok(cell(Value::String(self.render_bytes(&value)?)))
            }
            Builtin::Type => {
                let value = first();
                ensure_bound(&value)?;
                let name = self.object_type_name(&value.borrow());
                Ok(string_cell(&name))
            }
            Builtin::Time => {
                let seconds = SystemTime::now()
                    .duration_since(UNIX_EPOCH)
                    .map_err(|_| RuntimeError::new("system time is before the Unix epoch"))?
                    .as_secs();
                Ok(cell(Value::Integer(
                    i64::try_from(seconds).unwrap_or(i64::MAX),
                )))
            }
            Builtin::Eval => {
                let source = first();
                ensure_bound(&source)?;
                let source = match &**source.borrow() {
                    Value::String(source) => source.clone(),
                    _ => return Err(RuntimeError::new("eval expects a string")),
                };
                let mut eval_source = b"return ".to_vec();
                eval_source.extend_from_slice(&source);
                eval_source.push(b';');
                let ir = self.compile_ir(&eval_source, "<eval>", false)?;
                let previous_source =
                    std::mem::replace(&mut self.current_source, "<eval>".to_owned());
                let previous_source_bytes =
                    std::mem::replace(&mut self.current_source_bytes, b"<eval>".to_vec());
                let previous_constants = std::mem::replace(
                    &mut self.current_constants,
                    Rc::new(RefCell::new(HashMap::new())),
                );
                let execution = self.execute_ir_vm(&ir, environment);
                self.current_constants = previous_constants;
                self.current_source = previous_source;
                self.current_source_bytes = previous_source_bytes;
                Ok(execution?.value)
            }
            Builtin::Id => {
                let value = first();
                ensure_bound(&value)?;
                Ok(cell(Value::Integer(identity_id(&value))))
            }
            Builtin::Args => {
                if !arguments.is_empty() {
                    return Err(RuntimeError::new("args need no arguments"));
                }
                Ok(cell(list_value(
                    self.arguments
                        .iter()
                        .map(|argument| cell(Value::String(argument.clone())))
                        .collect(),
                )))
            }
            Builtin::Open => {
                if arguments.len() != 2 {
                    return Err(RuntimeError::new("function open need 2 arguments"));
                }
                ensure_bound(&arguments[0])?;
                ensure_bound(&arguments[1])?;
                let path = arguments[0].clone();
                let path = self.path_from_value(&path)?;
                let mode = match **arguments[1].borrow() {
                    Value::Integer(mode) => mode,
                    _ => return Err(RuntimeError::new("file mode should be integer")),
                };
                let mut options = OpenOptions::new();
                let append = mode & (1 << 0) != 0;
                let read = mode & (1 << 2) != 0;
                let write = mode & (1 << 3) != 0;
                let truncate = mode & (1 << 4) != 0 || (write && !read && !append);
                options
                    .append(append)
                    .read(read)
                    .write(write)
                    .truncate(truncate)
                    .create(append || truncate || (write && !read));
                let mut file = options.open(filesystem_call_path(&path)).ok();
                let seek_failed = mode & (1 << 5) != 0
                    && file
                        .as_mut()
                        .is_some_and(|stream| stream.seek(SeekFrom::End(0)).is_err());
                if seek_failed {
                    file = None;
                }
                let good = file.is_some();
                Ok(cell(Value::File(Rc::new(RefCell::new(RuntimeFile {
                    file,
                    read_buffer: Vec::new(),
                    read_cursor: 0,
                    write_buffer: Vec::new(),
                    good,
                    eof: false,
                    fail: !good,
                })))))
            }
            Builtin::CurrentPath => {
                if !arguments.is_empty() {
                    return Err(RuntimeError::new("function current_path need no arguments"));
                }
                let path = std::env::current_dir().map_err(|error| {
                    RuntimeError::plain_diagnostic(format!(
                        "filesystem error: cannot get current path: {}",
                        io_error_message(&error)
                    ))
                })?;
                Ok(cell(Value::Path(path)))
            }
            Builtin::IsDirectory => {
                if arguments.len() != 1 {
                    return Err(RuntimeError::new("function current_path need 1 argument"));
                }
                ensure_bound(&arguments[0])?;
                let path = self.path_from_value(&arguments[0])?;
                path_is_directory(&path).map(|value| cell(Value::Bool(value)))
            }
            Builtin::ReadDirectory => {
                if arguments.len() != 1 {
                    return Err(RuntimeError::new(
                        "function read_dir need only one argument",
                    ));
                }
                ensure_bound(&arguments[0])?;
                let path = self.path_from_value(&arguments[0])?;
                let mut entries = Vec::new();
                let directory = fs::read_dir(&path).map_err(|error| {
                    let mut message = format!(
                        "filesystem error: directory iterator cannot open directory: {} [",
                        io_error_message(&error),
                    )
                    .into_bytes();
                    message.extend(path_bytes(&path));
                    message.push(b']');
                    RuntimeError::plain_diagnostic_bytes(message)
                })?;
                for entry in directory {
                    let entry = entry.map_err(|error| {
                        RuntimeError::plain_diagnostic(format!(
                            "filesystem error: directory iterator cannot advance: {}",
                            io_error_message(&error)
                        ))
                    })?;
                    entries.push(cell(Value::Path(lexically_normal(&entry.path()))));
                }
                Ok(cell(list_value(entries)))
            }
            Builtin::Input => {
                let mut line = Vec::new();
                std::io::stdin().lock().read_until(b'\n', &mut line)?;
                if line.last() == Some(&b'\n') {
                    line.pop();
                }
                Ok(cell(Value::String(line)))
            }
            Builtin::Exit => {
                self.halted = true;
                Ok(cell(Value::None))
            }
            Builtin::CallWithCurrentContinuation => Err(RuntimeError::new(
                "call_with_current_continuation requires a VM context",
            )),
        }
    }

    fn call_method(
        &mut self,
        receiver: Cell,
        name: &str,
        arguments: Vec<Cell>,
    ) -> Result<Cell, RuntimeError> {
        ensure_bound(&receiver)?;
        let consumed_arguments = match (&**receiver.borrow(), name) {
            (Value::List(_), "push")
            | (Value::Dict(_), "at" | "erase")
            | (Value::File(_), "write" | "seekg" | "seekp") => 1,
            (Value::Dict(_), "insert") => 2,
            _ => 0,
        };
        for argument in arguments.iter().take(consumed_arguments) {
            ensure_bound(argument)?;
        }
        let mut borrowed = receiver.borrow_mut();
        match (&mut **borrowed, name) {
            (Value::List(items), "empty") => Ok(cell(Value::Bool(items.borrow().is_empty()))),
            (Value::List(items), "size") => Ok(cell(Value::Integer(items.borrow().len() as i64))),
            (Value::List(items), "push") => {
                let value = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                items
                    .borrow_mut()
                    .push(ListEntry::new(copy_variable(value)));
                Ok(cell(Value::None))
            }
            (Value::List(items), "pop") => items
                .borrow_mut()
                .pop()
                .map(|entry| entry.value)
                .ok_or_else(|| RuntimeError::new("pop from empty list")),
            (Value::List(items), "pop_front") => {
                let mut items = items.borrow_mut();
                if items.is_empty() {
                    Err(RuntimeError::new("pop from empty list"))
                } else {
                    Ok(items.remove(0).value)
                }
            }
            (Value::List(items), "front") => items
                .borrow()
                .first()
                .map(|entry| entry.value.clone())
                .ok_or_else(|| RuntimeError::new("front of empty list")),
            (Value::List(items), "back") => items
                .borrow()
                .last()
                .map(|entry| entry.value.clone())
                .ok_or_else(|| RuntimeError::new("back of empty list")),
            (Value::List(items), "clear") => {
                items.borrow_mut().clear();
                Ok(cell(Value::None))
            }
            (Value::List(items), "__iterator__") => {
                let next = items.borrow().first().map(|entry| entry.node.clone());
                Ok(cell(Value::ListIterator(Rc::new(ListIterator {
                    list: items.clone(),
                    next: Rc::new(RefCell::new(next)),
                }))))
            }
            (Value::ListIterator(iterator), "__has_next__") => {
                refresh_list_iterator(iterator);
                Ok(cell(Value::Bool(iterator.next.borrow().is_some())))
            }
            (Value::ListIterator(iterator), "__next__") => {
                refresh_list_iterator(iterator);
                let node = iterator
                    .next
                    .borrow()
                    .clone()
                    .ok_or_else(|| RuntimeError::new("iterator has no next value"))?;
                let list = iterator.list.borrow();
                let index = list
                    .iter()
                    .position(|entry| Rc::ptr_eq(&entry.node, &node))
                    .ok_or_else(|| RuntimeError::new("iterator has no next value"))?;
                let value = list[index].value.clone();
                *iterator.next.borrow_mut() = list.get(index + 1).map(|entry| entry.node.clone());
                Ok(value)
            }
            (Value::String(value), "size") => Ok(cell(Value::Integer(value.len() as i64))),
            (Value::String(value), "to_int") => {
                parse_integer_prefix(value).map(|value| cell(Value::Integer(value)))
            }
            (Value::Integer(value), "to_str") => Ok(string_cell(&value.to_string())),
            (Value::Dict(values), "empty") => Ok(cell(Value::Bool(values.borrow().is_empty()))),
            (Value::Dict(values), "size") => Ok(cell(Value::Integer(values.borrow().len() as i64))),
            (Value::Dict(values), "at") => {
                let key = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                dict_index(&mut values.borrow_mut(), key).map(copy_variable)
            }
            (Value::Dict(values), "insert") => {
                // Native methods ignore arguments beyond the values they consume.
                // Missing arguments still receive an explicit error.
                if arguments.len() < 2 {
                    return Err(RuntimeError::new(
                        "dict.insert expects at least 2 arguments",
                    ));
                }
                dict_insert(
                    &mut values.borrow_mut(),
                    copy_variable(arguments[0].clone()),
                    copy_variable(arguments[1].clone()),
                )?;
                Ok(cell(Value::None))
            }
            (Value::Dict(values), "erase") => {
                let key = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                let index = {
                    let values = values.borrow();
                    dict_find_index(&values, &key)?
                };
                if let Some(index) = index {
                    values.borrow_mut().remove(index);
                }
                Ok(cell(Value::None))
            }
            (Value::Dict(values), "clear") => {
                values.borrow_mut().clear();
                Ok(cell(Value::None))
            }
            (Value::File(file), method) => {
                let file = file.clone();
                drop(borrowed);
                self.call_file_method(&file, method, arguments)
            }
            (Value::Path(path), "is_directory") => {
                path_is_directory(path).map(|value| cell(Value::Bool(value)))
            }
            _ => Err(RuntimeError::new(format!("no member named {name}"))),
        }
    }

    fn unary(
        &mut self,
        operator: &str,
        operand: Cell,
        location: Location,
        _environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        ensure_bound(&operand).map_err(|error| error_at_location(error, location))?;
        let value = operand.borrow().value.clone();
        match (operator, value) {
            ("not" | "!", _) => Ok(cell(Value::Bool(!self.truthy(&operand, location)?))),
            ("-", Value::Integer(value)) => value
                .checked_neg()
                .map(|value| cell(Value::Integer(value)))
                .ok_or_else(|| RuntimeError::at(location, "integer overflow")),
            ("-", Value::Float(value)) => Ok(cell(Value::Float(-value))),
            ("~", Value::Integer(value)) => Ok(cell(Value::Integer(!value))),
            ("-", _) => Err(RuntimeError::at(location, "no neg method")),
            ("~", _) => Err(RuntimeError::at(location, "no bneg method")),
            _ => Err(RuntimeError::at(
                location,
                format!("no prefix operator {operator}"),
            )),
        }
    }

    fn binary(
        &mut self,
        operator: &str,
        left: Cell,
        right: Cell,
        location: Location,
        _environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        ensure_bound(&left).map_err(|error| error_at_location(error, location))?;
        ensure_bound(&right).map_err(|error| error_at_location(error, location))?;
        let left_value = left.borrow().value.clone();
        let right_value = right.borrow().value.clone();
        let result = match (operator, left_value, right_value) {
            ("+", Value::Integer(left), Value::Integer(right)) => Value::Integer(
                left.checked_add(right)
                    .ok_or_else(|| RuntimeError::at(location, "integer overflow"))?,
            ),
            ("-", Value::Integer(left), Value::Integer(right)) => Value::Integer(
                left.checked_sub(right)
                    .ok_or_else(|| RuntimeError::at(location, "integer overflow"))?,
            ),
            ("*", Value::Integer(left), Value::Integer(right)) => Value::Integer(
                left.checked_mul(right)
                    .ok_or_else(|| RuntimeError::at(location, "integer overflow"))?,
            ),
            ("/", Value::Integer(_), Value::Integer(0))
            | ("%", Value::Integer(_), Value::Integer(0)) => {
                return Err(RuntimeError::at(location, "integer division by zero"));
            }
            ("/", Value::Integer(left), Value::Integer(right)) => Value::Integer(
                left.checked_div(right)
                    .ok_or_else(|| RuntimeError::at(location, "integer overflow"))?,
            ),
            ("%", Value::Integer(left), Value::Integer(right)) => Value::Integer(
                left.checked_rem(right)
                    .ok_or_else(|| RuntimeError::at(location, "integer overflow"))?,
            ),
            ("+", Value::Float(left), Value::Float(right)) => Value::Float(left + right),
            ("-", Value::Float(left), Value::Float(right)) => Value::Float(left - right),
            ("*", Value::Float(left), Value::Float(right)) => Value::Float(left * right),
            ("/", Value::Float(left), Value::Float(right)) => Value::Float(left / right),
            ("+", Value::String(left), Value::String(right)) => {
                let mut combined = left;
                combined.extend(right);
                Value::String(combined)
            }
            ("+", Value::List(left), Value::List(right)) => {
                let mut combined: Vec<_> = left
                    .borrow()
                    .iter()
                    .map(|entry| entry.value.clone())
                    .collect();
                combined.extend(right.borrow().iter().map(|entry| entry.value.clone()));
                list_value(combined)
            }
            ("&", Value::Integer(left), Value::Integer(right)) => Value::Integer(left & right),
            ("|", Value::Integer(left), Value::Integer(right)) => Value::Integer(left | right),
            ("^", Value::Integer(left), Value::Integer(right)) => Value::Integer(left ^ right),
            ("<<", Value::Integer(left), Value::Integer(right)) => {
                let distance = u32::try_from(right)
                    .ok()
                    .filter(|distance| *distance < i64::BITS)
                    .ok_or_else(|| RuntimeError::at(location, "invalid shift count"))?;
                let shifted = (i128::from(left)) << distance;
                Value::Integer(
                    i64::try_from(shifted)
                        .map_err(|_| RuntimeError::at(location, "integer overflow"))?,
                )
            }
            (">>", Value::Integer(left), Value::Integer(right)) => {
                let distance = u32::try_from(right)
                    .ok()
                    .filter(|distance| *distance < i64::BITS)
                    .ok_or_else(|| RuntimeError::at(location, "invalid shift count"))?;
                Value::Integer(left >> distance)
            }
            ("and", _, _) => {
                Value::Bool(self.truthy(&left, location)? && self.truthy(&right, location)?)
            }
            ("or", _, _) => {
                Value::Bool(self.truthy(&left, location)? || self.truthy(&right, location)?)
            }
            ("is", _, _) => Value::Bool(values_identical(&left, &right)),
            ("=", Value::Integer(left), Value::Integer(right)) => Value::Bool(left == right),
            ("!=", Value::Integer(left), Value::Integer(right)) => Value::Bool(left != right),
            ("=", Value::Float(left), Value::Float(right)) => Value::Bool(left == right),
            ("!=", Value::Float(left), Value::Float(right)) => Value::Bool(left != right),
            ("=" | "!=", Value::Integer(_), _) | ("=" | "!=", Value::Float(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("=", _, _) => Value::Bool(values_equal(&left, &right)),
            ("!=", _, _) => Value::Bool(!values_equal(&left, &right)),
            ("<", Value::Integer(left), Value::Integer(right)) => Value::Bool(left < right),
            ("<=", Value::Integer(left), Value::Integer(right)) => Value::Bool(left <= right),
            (">", Value::Integer(left), Value::Integer(right)) => Value::Bool(left >= right),
            (">=", Value::Integer(left), Value::Integer(right)) => Value::Bool(left > right),
            ("<", Value::Float(left), Value::Float(right)) => Value::Bool(left < right),
            ("<=", Value::Float(left), Value::Float(right)) => Value::Bool(left <= right),
            (">", Value::Float(left), Value::Float(right)) => Value::Bool(left >= right),
            (">=", Value::Float(left), Value::Float(right)) => Value::Bool(left > right),
            ("<", Value::String(left), Value::String(right)) => Value::Bool(left < right),
            ("<=", Value::String(left), Value::String(right)) => Value::Bool(left <= right),
            // Greater comparisons use the reversed less-than operations.
            (">", Value::String(left), Value::String(right)) => Value::Bool(left >= right),
            (">=", Value::String(left), Value::String(right)) => Value::Bool(left > right),
            ("<" | "<=", Value::String(_), _) => Value::Bool(true),
            (">" | ">=", _, Value::String(_)) => Value::Bool(true),
            ("<" | "<=", Value::Integer(_) | Value::Float(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            (
                ">" | ">=",
                Value::Integer(_) | Value::Float(_),
                Value::Integer(_) | Value::Float(_),
            ) => return Err(RuntimeError::at(location, "no match method")),
            (">" | ">=", _, Value::Integer(_) | Value::Float(_)) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("<", _, _) => return Err(RuntimeError::at(location, "no clt method")),
            ("<=", _, _) => return Err(RuntimeError::at(location, "no cle method")),
            (">", _, _) => return Err(RuntimeError::at(location, "no cle method")),
            (">=", _, _) => return Err(RuntimeError::at(location, "no clt method")),
            ("+", Value::Integer(_) | Value::Float(_) | Value::String(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("+", Value::List(_), _) => {
                return Err(RuntimeError::at(location, "expected"));
            }
            ("+", _, _) => return Err(RuntimeError::at(location, "no add method")),
            ("-", Value::Integer(_) | Value::Float(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("-", _, _) => return Err(RuntimeError::at(location, "no sub method")),
            ("*", Value::Integer(_) | Value::Float(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("*", _, _) => return Err(RuntimeError::at(location, "no mul method")),
            ("/", Value::Integer(_) | Value::Float(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("/", _, _) => return Err(RuntimeError::at(location, "no div method")),
            ("%", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("%", _, _) => return Err(RuntimeError::at(location, "no mod method")),
            ("&", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("&", _, _) => return Err(RuntimeError::at(location, "no band method")),
            ("|", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("|", _, _) => return Err(RuntimeError::at(location, "no bor method")),
            ("^", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("^", _, _) => return Err(RuntimeError::at(location, "no bxor method")),
            ("<<", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            ("<<", _, _) => return Err(RuntimeError::at(location, "no bls method")),
            (">>", Value::Integer(_), _) => {
                return Err(RuntimeError::at(location, "no match method"));
            }
            (">>", _, _) => return Err(RuntimeError::at(location, "no brs method")),
            _ => {
                return Err(RuntimeError::at(
                    location,
                    format!("no infix operator {operator}"),
                ));
            }
        };
        Ok(cell(result))
    }

    fn match_equal(
        &mut self,
        value: Cell,
        key: Cell,
        location: Location,
        environment: &Environment,
    ) -> Result<bool, RuntimeError> {
        let result = self.binary("=", value, key, location, environment)?;
        self.truthy(&result, location)
    }

    fn reference(&mut self, value: Cell) -> Result<Cell, RuntimeError> {
        Ok(value)
    }

    fn truthy(&mut self, value: &Cell, location: Location) -> Result<bool, RuntimeError> {
        if let Err(mut error) = ensure_bound(value) {
            error.location = Some(location);
            return Err(error);
        }
        let result = match &**value.borrow() {
            Value::Unbound(_) => unreachable!("unbound values are rejected above"),
            Value::Bool(value) => *value,
            Value::Integer(value) => *value != 0,
            Value::Float(value) => *value != 0.0,
            Value::String(value) => !value.is_empty(),
            Value::List(value) => !value.borrow().is_empty(),
            Value::Dict(value) => !value.borrow().is_empty(),
            Value::None
            | Value::File(_)
            | Value::Path(_)
            | Value::ListIterator(_)
            | Value::Continuation(_)
            | Value::IrFunction(_)
            | Value::Namespace(_)
            | Value::Enum(_)
            | Value::Class(_)
            | Value::Instance { .. }
            | Value::UserIrMethod { .. }
            | Value::Builtin(_)
            | Value::BoundMethod { .. }
            | Value::IrThunk(_) => {
                return Err(RuntimeError::at(location, "cannot translate to bool"));
            }
        };
        Ok(result)
    }

    fn stringify(&mut self, value: &Cell) -> Result<String, RuntimeError> {
        Ok(String::from_utf8_lossy(&self.render_bytes(value)?).into_owned())
    }

    fn render_bytes(&mut self, value: &Cell) -> Result<Vec<u8>, RuntimeError> {
        object_bytes(value)
    }

    fn call_file_method(
        &mut self,
        runtime_file: &Rc<RefCell<RuntimeFile>>,
        method: &str,
        arguments: Vec<Cell>,
    ) -> Result<Cell, RuntimeError> {
        let mut runtime_file = runtime_file.borrow_mut();
        match method {
            "good" => Ok(cell(Value::Bool(runtime_file.good))),
            "eof" => Ok(cell(Value::Bool(runtime_file.eof))),
            "close" => {
                if runtime_file.file.is_none() {
                    runtime_file.fail = true;
                    runtime_file.good = false;
                } else {
                    if runtime_file.flush_write_buffer().is_err() {
                        runtime_file.fail = true;
                        runtime_file.good = false;
                    }
                    runtime_file.file.take();
                }
                Ok(cell(Value::None))
            }
            "flush" => {
                if runtime_file.good {
                    let failed = runtime_file.flush_write_buffer().is_err()
                        || runtime_file
                            .file
                            .as_mut()
                            .is_none_or(|file| file.flush().is_err());
                    if failed {
                        runtime_file.fail = true;
                        runtime_file.good = false;
                    }
                }
                Ok(cell(Value::None))
            }
            "read" => {
                if !runtime_file.good {
                    return Ok(cell(Value::String(vec![u8::MAX])));
                }
                if runtime_file.file.is_none() {
                    runtime_file.eof = true;
                    runtime_file.fail = true;
                    runtime_file.good = false;
                    return Ok(cell(Value::String(vec![u8::MAX])));
                }
                let byte = runtime_file.read_buffered_byte()?;
                runtime_file.eof = byte.is_none();
                runtime_file.good = byte.is_some();
                runtime_file.fail = byte.is_none();
                let value = vec![byte.unwrap_or(u8::MAX)];
                Ok(cell(Value::String(value)))
            }
            "readline" => {
                if !runtime_file.good {
                    return Ok(string_cell(""));
                }
                if runtime_file.file.is_none() {
                    runtime_file.eof = true;
                    runtime_file.fail = true;
                    runtime_file.good = false;
                    return Ok(string_cell(""));
                }
                let mut line = Vec::new();
                let mut reached_eof = false;
                loop {
                    let Some(byte) = runtime_file.read_buffered_byte()? else {
                        reached_eof = true;
                        break;
                    };
                    if byte == b'\n' {
                        break;
                    }
                    line.push(byte);
                }
                runtime_file.eof = reached_eof;
                runtime_file.good = !reached_eof;
                runtime_file.fail = reached_eof && line.is_empty();
                Ok(cell(Value::String(line)))
            }
            "write" => {
                let value = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                let bytes = match &**value.borrow() {
                    Value::String(bytes) => bytes.clone(),
                    _ => return Err(RuntimeError::new("file write expects a string")),
                };
                if runtime_file.good {
                    if runtime_file.file.is_some() {
                        if runtime_file.write_buffered(&bytes).is_err() {
                            runtime_file.fail = true;
                            runtime_file.good = false;
                        }
                    } else {
                        runtime_file.fail = true;
                        runtime_file.good = false;
                    }
                }
                Ok(cell(Value::None))
            }
            "tellg" => {
                if !runtime_file.good {
                    runtime_file.fail = true;
                    return Ok(cell(Value::Integer(-1)));
                }
                if runtime_file.file.is_none() {
                    return Ok(cell(Value::Integer(-1)));
                }
                let position = runtime_file.logical_position()?;
                Ok(cell(Value::Integer(
                    i64::try_from(position).unwrap_or(i64::MAX),
                )))
            }
            "tellp" => {
                if runtime_file.fail {
                    return Ok(cell(Value::Integer(-1)));
                }
                if runtime_file.file.is_none() {
                    return Ok(cell(Value::Integer(-1)));
                }
                let position = runtime_file.logical_position()?;
                Ok(cell(Value::Integer(
                    i64::try_from(position).unwrap_or(i64::MAX),
                )))
            }
            "seekg" | "seekp" => {
                let position = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::Integer(0)));
                let position = match **position.borrow() {
                    Value::Integer(position) => position,
                    _ => return Err(RuntimeError::new("seek position should be integer")),
                };
                if method == "seekg" && runtime_file.eof {
                    runtime_file.eof = false;
                    runtime_file.good = !runtime_file.fail;
                }
                let can_seek = if method == "seekp" {
                    !runtime_file.fail
                } else {
                    runtime_file.good
                };
                if can_seek {
                    if runtime_file.flush_write_buffer().is_err() || position < 0 {
                        runtime_file.fail = true;
                        runtime_file.good = false;
                    } else if let Some(file) = runtime_file.file.as_mut() {
                        if file.seek(SeekFrom::Start(position as u64)).is_err() {
                            runtime_file.fail = true;
                            runtime_file.good = false;
                        } else {
                            runtime_file.discard_read_buffer();
                        }
                    } else {
                        runtime_file.fail = true;
                        runtime_file.good = false;
                    }
                }
                Ok(cell(Value::None))
            }
            _ => Err(RuntimeError::new(format!("no member named {method}"))),
        }
    }

    fn path_from_value(&mut self, value: &Cell) -> Result<PathBuf, RuntimeError> {
        let mut path = match &**value.borrow() {
            Value::Path(path) => path.clone(),
            Value::String(path) => path_from_bytes(path),
            _ => PathBuf::from(self.stringify(value)?),
        };
        if path.is_relative() {
            let working_directory = self
                .directories
                .first()
                .cloned()
                .unwrap_or_else(|| PathBuf::from("."));
            path = working_directory.join(path);
        }
        Ok(path)
    }

    fn load_module(
        &mut self,
        parts: &[ModulePart],
        caller_environment: &Environment,
    ) -> Result<LoadedModule, RuntimeError> {
        let Some(first) = parts.first() else {
            return Err(RuntimeError::new("empty module path"));
        };
        let module = match first {
            ModulePart::Name(name) => {
                if let Some(path) = self.resolve_named_module(name)? {
                    self.load_module_path(&path, caller_environment)?
                } else {
                    LoadedModule {
                        value: self.load_standard_module(name)?,
                        root_return: None,
                        deferred_cache_key: None,
                    }
                }
            }
            ModulePart::Path(path) => {
                let requested = path_from_bytes(path);
                let resolved = self.current_directory().join(&requested);
                self.load_module_path(&resolved, caller_environment)?
            }
        };
        let value = self.module_member_path(copy_variable(module.value), &parts[1..])?;
        Ok(LoadedModule {
            value,
            root_return: module.root_return,
            deferred_cache_key: module.deferred_cache_key,
        })
    }

    fn module_member_path(
        &mut self,
        mut value: Cell,
        parts: &[ModulePart],
    ) -> Result<Cell, RuntimeError> {
        for part in parts {
            let ModulePart::Name(name) = part else {
                return Err(RuntimeError::new(
                    "module denoted by path must be the top module",
                ));
            };
            value = self.member(value, name, Location { line: 0, column: 0 })?;
        }
        Ok(value)
    }

    fn resolve_named_module(&self, name: &str) -> Result<Option<PathBuf>, RuntimeError> {
        let current = self.current_directory();
        let roots = [current, PathBuf::from("/usr/local/lib/anole")];
        for root in roots {
            let mut file_name = symbol_to_bytes(name);
            file_name.extend_from_slice(b".anole");
            let file = root.join(path_from_bytes(&file_name));
            if path_is_regular_file(&file)? {
                return Ok(Some(file));
            }
            let directory = root.join(path_from_bytes(&symbol_to_bytes(name)));
            if path_is_directory(&directory)? {
                return Ok(Some(directory.join("__init__.anole")));
            }
        }
        Ok(None)
    }

    fn load_module_path(
        &mut self,
        path: &Path,
        caller_environment: &Environment,
    ) -> Result<LoadedModule, RuntimeError> {
        // Cache keys are lexical paths and intentionally do not resolve symlinks.
        let normalized = lexically_normal(path);
        let cache_key = path_bytes(&normalized);
        if let Some(module) = self.modules.get(&cache_key) {
            return Ok(LoadedModule {
                value: module.clone(),
                root_return: None,
                deferred_cache_key: None,
            });
        }
        let path = normalized.clone();
        let module_environment = Env::child(&self.builtins);
        let module = cell(Value::Namespace(module_environment.clone()));
        self.directories.push(lexical_path_parent(&path));
        let trailing_separator = path_ends_with_directory_separator(&path);
        let source_name = if trailing_separator {
            String::new()
        } else {
            path.file_name()
                .and_then(|name| name.to_str())
                .unwrap_or("<module>")
                .to_owned()
        };
        let source_name_bytes = if trailing_separator {
            Vec::new()
        } else {
            path.file_name()
                .map(|name| path_bytes(Path::new(name)))
                .unwrap_or_else(|| b"<module>".to_vec())
        };
        let previous_source = std::mem::replace(&mut self.current_source, source_name);
        let previous_source_bytes =
            std::mem::replace(&mut self.current_source_bytes, source_name_bytes);
        let previous_constants = std::mem::replace(
            &mut self.current_constants,
            Rc::new(RefCell::new(HashMap::new())),
        );
        let ir_path = sidecar_path(&path, ".ir");
        let execution = (|| {
            if let Some(ir) = fresh_legacy_ir(&path, &ir_path)?
                && self.supports_cached_ir(&ir)
            {
                let outcome = self.execute_ir_vm(&ir, &module_environment)?;
                let root_return = outcome.root_returned.then_some(outcome.value);
                let exported = if root_return.is_some() {
                    caller_environment.clone()
                } else {
                    outcome.environment
                };
                **module.borrow_mut() = Value::Namespace(exported);
                Ok(root_return)
            } else {
                // Direct path imports of directories create an empty module;
                // only named directory imports resolve to `__init__.anole`.
                let filesystem_path = filesystem_call_path(&path);
                let source = if filesystem_path.is_dir() {
                    File::open(&filesystem_path).map(|_| Vec::new())
                } else {
                    fs::read(&filesystem_path)
                }
                .map_err(|_| {
                    let mut message = b"cannot open file ".to_vec();
                    message.extend(path_bytes(&path));
                    RuntimeError::new_bytes(message)
                })?;
                let outcome = self.execute_incremental_ir(
                    &source,
                    &path.display().to_string(),
                    false,
                    module_environment.clone(),
                )?;
                let exported = if outcome.root_return.is_some() {
                    caller_environment.clone()
                } else {
                    outcome.environment
                };
                **module.borrow_mut() = Value::Namespace(exported);
                if !self.halted {
                    let _ = outcome.ir.write_to(&filesystem_call_path(&ir_path));
                }
                Ok(outcome.root_return)
            }
        })();
        let displayed_path = path.display().to_string();
        let raw_path = path_bytes(&path);
        let execution = execution.map_err(|error: RuntimeError| {
            self.attach_error(error.replace_diagnostic_source(&displayed_path, &raw_path))
        });
        self.current_constants = previous_constants;
        self.current_source = previous_source;
        self.current_source_bytes = previous_source_bytes;
        self.directories.pop();
        let root_return = execution?;
        let deferred_cache_key = root_return.as_ref().map(|_| cache_key.clone());
        if deferred_cache_key.is_none() {
            self.modules.insert(cache_key, module.clone());
        }
        Ok(LoadedModule {
            value: module,
            root_return,
            deferred_cache_key,
        })
    }

    fn load_standard_module(&mut self, name: &str) -> Result<Cell, RuntimeError> {
        let cache_key = format!("<embedded>/{name}").into_bytes();
        if let Some(module) = self.modules.get(&cache_key) {
            return Ok(module.clone());
        }
        let environment = Env::child(&self.builtins);
        let module = cell(Value::Namespace(environment.clone()));
        let result = if name == "coroutine" {
            const SOURCE: &str = include_str!("stdlib/coroutine.anole");
            let ir = self.compile_ir(
                SOURCE.as_bytes(),
                "<embedded>/coroutine/__init__.anole",
                false,
            )?;
            let previous_source =
                std::mem::replace(&mut self.current_source, "__init__.anole".to_owned());
            let previous_source_bytes =
                std::mem::replace(&mut self.current_source_bytes, b"__init__.anole".to_vec());
            let previous_constants = std::mem::replace(
                &mut self.current_constants,
                Rc::new(RefCell::new(HashMap::new())),
            );
            let execution = self.execute_ir_vm(&ir, &environment);
            self.current_constants = previous_constants;
            self.current_source = previous_source;
            self.current_source_bytes = previous_source_bytes;
            let outcome = execution?;
            **module.borrow_mut() = Value::Namespace(outcome.environment);
            Ok(module)
        } else {
            self.initialize_standard_module(name, environment, module)
        };
        if let Ok(module) = &result {
            self.modules.insert(cache_key, module.clone());
        }
        result
    }

    fn initialize_standard_module(
        &mut self,
        name: &str,
        environment: Environment,
        module: Cell,
    ) -> Result<Cell, RuntimeError> {
        match name {
            "env" => {
                Env::define(
                    &environment,
                    "__args".to_owned(),
                    cell(Value::Builtin(Builtin::Args)),
                );
                let environment = self.execute_embedded_module_source(
                    include_str!("stdlib/env.anole"),
                    "<embedded>/env/__init__.anole",
                    environment,
                )?;
                **module.borrow_mut() = Value::Namespace(environment);
            }
            "file" => {
                Env::define(
                    &environment,
                    "__open".to_owned(),
                    cell(Value::Builtin(Builtin::Open)),
                );
                let environment = self.execute_embedded_module_source(
                    include_str!("stdlib/file.anole"),
                    "<embedded>/file/__init__.anole",
                    environment,
                )?;
                **module.borrow_mut() = Value::Namespace(environment);
            }
            "os" => {
                let path = Env::child(&self.builtins);
                Env::define(
                    &path,
                    "__current_path".to_owned(),
                    cell(Value::Builtin(Builtin::CurrentPath)),
                );
                Env::define(
                    &path,
                    "__is_directory".to_owned(),
                    cell(Value::Builtin(Builtin::IsDirectory)),
                );
                let path = self.execute_embedded_module_source(
                    include_str!("stdlib/os_path.anole"),
                    "<embedded>/os/path/__init__.anole",
                    path,
                )?;
                let read_directory = Env::child(&self.builtins);
                Env::define(
                    &read_directory,
                    "__read_dir".to_owned(),
                    cell(Value::Builtin(Builtin::ReadDirectory)),
                );
                let read_directory = self.execute_embedded_module_source(
                    include_str!("stdlib/os_read_dir.anole"),
                    "<embedded>/os/read_dir/__init__.anole",
                    read_directory,
                )?;
                Env::define(
                    &environment,
                    "path".to_owned(),
                    cell(Value::Namespace(path)),
                );
                Env::define(
                    &environment,
                    "read_dir".to_owned(),
                    cell(Value::Namespace(read_directory)),
                );
            }
            "debug" => {}
            _ => return Err(symbol_runtime_error(b"no module named ", name, b"")),
        }
        Ok(module)
    }

    fn execute_embedded_module_source(
        &mut self,
        source: &str,
        name: &str,
        environment: Environment,
    ) -> Result<Environment, RuntimeError> {
        let ir = self.compile_ir(source.as_bytes(), name, false)?;
        let previous_source = std::mem::replace(&mut self.current_source, "__init__.anole".into());
        let previous_source_bytes =
            std::mem::replace(&mut self.current_source_bytes, b"__init__.anole".to_vec());
        let previous_constants = std::mem::replace(
            &mut self.current_constants,
            Rc::new(RefCell::new(HashMap::new())),
        );
        let execution = self.execute_ir_vm(&ir, &environment);
        self.current_constants = previous_constants;
        self.current_source = previous_source;
        self.current_source_bytes = previous_source_bytes;
        execution.map(|outcome| outcome.environment)
    }

    fn current_directory(&self) -> PathBuf {
        self.directories
            .last()
            .cloned()
            .unwrap_or_else(|| PathBuf::from("."))
    }

    fn emit_bytes(&mut self, value: &[u8]) -> Result<(), RuntimeError> {
        if self.stream_output {
            let mut stdout = std::io::stdout().lock();
            stdout.write_all(value)?;
            stdout.flush()?;
        } else {
            self.output.push_str(&String::from_utf8_lossy(value));
        }
        Ok(())
    }

    fn attach_error(&self, mut error: RuntimeError) -> RuntimeError {
        if !error.has_source_or_diagnostic() {
            error.attach_source(
                self.current_source.clone(),
                self.current_source_bytes.clone(),
                self.call_frames.iter().take(66).cloned().collect(),
            );
        }
        error
    }
}

fn cell(value: Value) -> Cell {
    Rc::new(RefCell::new(ValueSlot::new(value)))
}

fn refresh_list_iterator(iterator: &ListIterator) {
    let next_was_removed = iterator.next.borrow().as_ref().is_some_and(|next| {
        !iterator
            .list
            .borrow()
            .iter()
            .any(|entry| Rc::ptr_eq(&entry.node, next))
    });
    if next_was_removed {
        *iterator.next.borrow_mut() = None;
    }
}

fn previous_import_name(context: &VmContext) -> String {
    context
        .pc
        .checked_sub(1)
        .and_then(|pc| context.code.borrow().instructions().get(pc).cloned())
        .and_then(|instruction| match instruction.operand {
            IrOperand::String(name) => Some(name),
            _ => None,
        })
        .unwrap_or_default()
}

fn vm_pop(context: &VmContext) -> Result<Cell, RuntimeError> {
    context
        .stack
        .borrow_mut()
        .pop()
        .ok_or_else(|| RuntimeError::new("invalid cached IR stack"))
}

fn ir_target(target: u64) -> Result<usize, RuntimeError> {
    usize::try_from(target).map_err(|_| RuntimeError::new("invalid cached IR jump target"))
}

fn fresh_legacy_ir(source: &Path, ir: &Path) -> Result<Option<LegacyIr>, RuntimeError> {
    if !path_is_regular_file(ir)? {
        return Ok(None);
    }
    let ir_modified = path_modified_time(ir)?;
    let source_modified = path_modified_time(source)?;
    if ir_modified < source_modified {
        return Ok(None);
    }
    match LegacyIr::read_from(&filesystem_call_path(ir)) {
        Ok(Ok(code)) => Ok(code),
        Ok(Err(_)) => Err(invalid_constant_tag_error()),
        Err(_) => Ok(None),
    }
}

fn invalid_constant_tag_error() -> RuntimeError {
    RuntimeError::plain_diagnostic("WTF, you want me to eat shit?!")
}

fn lexically_normal(path: &Path) -> PathBuf {
    if path.as_os_str().is_empty() {
        return PathBuf::new();
    }
    let preserve_trailing_separator = path_preserves_trailing_separator(path);
    let mut normalized = PathBuf::new();
    for component in path.components() {
        match component {
            Component::Prefix(_) | Component::RootDir | Component::Normal(_) => {
                normalized.push(component.as_os_str());
            }
            Component::CurDir => {}
            Component::ParentDir => {
                let can_pop = matches!(
                    normalized.components().next_back(),
                    Some(Component::Normal(_))
                );
                if can_pop {
                    normalized.pop();
                } else if !normalized.has_root() {
                    normalized.push(component.as_os_str());
                }
            }
        }
    }
    if preserve_trailing_separator
        && matches!(
            normalized.components().next_back(),
            Some(Component::Normal(_))
        )
    {
        normalized.push("");
    }
    if normalized.as_os_str().is_empty() {
        normalized.push(".");
    }
    normalized
}

#[cfg(unix)]
fn path_preserves_trailing_separator(path: &Path) -> bool {
    let bytes = path.as_os_str().as_bytes();
    if path_ends_with_directory_separator(path) {
        return true;
    }
    matches!(
        bytes.rsplit(|byte| *byte == b'/').next(),
        Some(b"." | b"..")
    )
}

#[cfg(not(unix))]
fn path_preserves_trailing_separator(path: &Path) -> bool {
    let path = path.as_os_str().to_string_lossy();
    path_ends_with_directory_separator(Path::new(path.as_ref()))
        || matches!(path.rsplit(['/', '\\']).next(), Some("." | ".."))
}

#[cfg(unix)]
fn path_ends_with_directory_separator(path: &Path) -> bool {
    path.as_os_str().as_bytes().last() == Some(&b'/')
}

#[cfg(not(unix))]
fn path_ends_with_directory_separator(path: &Path) -> bool {
    path.as_os_str().to_string_lossy().ends_with(['/', '\\'])
}

#[cfg(unix)]
fn lexical_path_parent(path: &Path) -> PathBuf {
    if path_ends_with_directory_separator(path) {
        let bytes = path.as_os_str().as_bytes();
        let mut end = bytes.len();
        while end > 1 && bytes[end - 1] == b'/' {
            end -= 1;
        }
        return path_from_bytes(&bytes[..end]);
    }
    path.parent()
        .unwrap_or_else(|| Path::new("."))
        .to_path_buf()
}

#[cfg(not(unix))]
fn lexical_path_parent(path: &Path) -> PathBuf {
    if path_ends_with_directory_separator(path) {
        let path = path.as_os_str().to_string_lossy();
        let trimmed = path.trim_end_matches(['/', '\\']);
        if !trimmed.is_empty() {
            return PathBuf::from(trimmed);
        }
    }
    path.parent()
        .unwrap_or_else(|| Path::new("."))
        .to_path_buf()
}

fn string_cell(value: &str) -> Cell {
    cell(Value::String(value.as_bytes().to_vec()))
}

fn parse_integer_prefix(value: &[u8]) -> Result<i64, RuntimeError> {
    let mut index = 0;
    while value.get(index).is_some_and(u8::is_ascii_whitespace) {
        index += 1;
    }
    let start = index;
    if value
        .get(index)
        .is_some_and(|byte| matches!(byte, b'+' | b'-'))
    {
        index += 1;
    }
    let digits = index;
    while value.get(index).is_some_and(u8::is_ascii_digit) {
        index += 1;
    }
    if index == digits {
        return Err(RuntimeError::plain_diagnostic("stoll"));
    }
    std::str::from_utf8(&value[start..index])
        .ok()
        .and_then(|value| value.parse().ok())
        .ok_or_else(|| RuntimeError::plain_diagnostic("stoll"))
}

fn constant_pool(ir: &LegacyIr) -> ConstantPool {
    let pool = Rc::new(RefCell::new(HashMap::new()));
    for (key, constant) in ir.constants() {
        let value = match constant {
            IrConstant::Integer(value) => Value::Integer(*value),
            IrConstant::Float(value) => Value::Float(*value),
            IrConstant::String(value) => Value::String(value.clone()),
        };
        pool.borrow_mut().insert(key.clone(), cell(value));
    }
    pool
}

fn populate_constants(constants: &ConstantPool, code: &Rc<RefCell<LegacyIr>>) {
    for (key, constant) in code.borrow().constants() {
        let value = match constant {
            IrConstant::Integer(value) => Value::Integer(*value),
            IrConstant::Float(value) => Value::Float(*value),
            IrConstant::String(value) => Value::String(value.clone()),
        };
        constants
            .borrow_mut()
            .entry(key.clone())
            .or_insert_with(|| cell(value));
    }
}

fn function_call_error(
    parent: &VmContext,
    function: &IrFunction,
    pc: usize,
    call_location: Location,
    message: Vec<u8>,
) -> RuntimeError {
    let mut trace = parent.trace.clone();
    trace.push((parent.source_bytes.clone(), call_location));
    RuntimeError {
        location: function.code.mapped_location(pc),
        message: String::from_utf8_lossy(&message).into_owned(),
        details: Box::new(RuntimeErrorDetails {
            message_bytes: Some(message),
            source: Some(function.source.clone()),
            source_bytes: Some(function.source_bytes.clone()),
            trace,
            ..RuntimeErrorDetails::default()
        }),
    }
}

fn list_value(values: Vec<Cell>) -> Value {
    Value::List(Rc::new(RefCell::new(
        values.into_iter().map(ListEntry::new).collect(),
    )))
}

fn native_method_consumed_arguments(receiver: &Cell, name: &str) -> usize {
    match (&**receiver.borrow(), name) {
        (Value::List(_), "push")
        | (Value::Dict(_), "at" | "erase")
        | (Value::File(_), "write" | "seekg" | "seekp") => 1,
        (Value::Dict(_), "insert") => 2,
        _ => 0,
    }
}

fn native_method_pushes_result(receiver: &Cell, name: &str, operand_stack_is_empty: bool) -> bool {
    let method_pushes_result = !matches!(
        (&**receiver.borrow(), name),
        (
            Value::File(_),
            "close" | "flush" | "write" | "seekg" | "seekp"
        )
    );
    // Void methods leave an existing operand in place. Use `none` only when
    // the expression would otherwise leave the operand stack empty.
    method_pushes_result || operand_stack_is_empty
}

fn dict_value(values: Vec<DictEntry>) -> Value {
    Value::Dict(Rc::new(RefCell::new(values)))
}

fn copy_variable(value: Cell) -> Cell {
    Rc::new(RefCell::new(value.borrow().clone()))
}

fn name_unbound(value: &Cell, name: &str) {
    let mut value = value.borrow_mut();
    if let Value::Unbound(current_name) = &mut **value {
        *current_name = name.to_owned();
    }
}

fn ensure_bound(value: &Cell) -> Result<(), RuntimeError> {
    if let Value::Unbound(name) = &**value.borrow() {
        Err(symbol_runtime_error(
            b"var named ",
            name,
            b" doesn't reference to any object",
        ))
    } else {
        Ok(())
    }
}

fn symbol_runtime_error(prefix: &[u8], symbol: &str, suffix: &[u8]) -> RuntimeError {
    let mut message = Vec::with_capacity(prefix.len() + symbol.len() + suffix.len());
    message.extend_from_slice(prefix);
    message.extend(symbol_to_bytes(symbol));
    message.extend_from_slice(suffix);
    RuntimeError::new_bytes(message)
}

fn error_at_location(mut error: RuntimeError, location: Location) -> RuntimeError {
    if error.location.is_none() && !error.has_diagnostic() {
        error.location = Some(location);
    }
    error
}

fn io_error_message(error: &std::io::Error) -> String {
    let message = error.to_string();
    if let Some(index) = message.rfind(" (os error ")
        && message.ends_with(')')
    {
        message[..index].to_owned()
    } else {
        message
    }
}

fn path_is_directory(path: &Path) -> Result<bool, RuntimeError> {
    path_metadata(path).map(|metadata| metadata.is_some_and(|metadata| metadata.is_dir()))
}

fn path_is_regular_file(path: &Path) -> Result<bool, RuntimeError> {
    path_metadata(path).map(|metadata| metadata.is_some_and(|metadata| metadata.is_file()))
}

fn path_metadata(path: &Path) -> Result<Option<fs::Metadata>, RuntimeError> {
    match fs::metadata(filesystem_call_path(path)) {
        Ok(metadata) => Ok(Some(metadata)),
        Err(error)
            if matches!(
                error.kind(),
                std::io::ErrorKind::NotFound | std::io::ErrorKind::NotADirectory
            ) =>
        {
            Ok(None)
        }
        Err(error) => {
            let mut message =
                format!("filesystem error: status: {} [", io_error_message(&error)).into_bytes();
            message.extend(path_bytes(path));
            message.push(b']');
            Err(RuntimeError::plain_diagnostic_bytes(message))
        }
    }
}

fn path_modified_time(path: &Path) -> Result<SystemTime, RuntimeError> {
    fs::metadata(filesystem_call_path(path))
        .and_then(|metadata| metadata.modified())
        .map_err(|error| {
            let mut message = format!(
                "filesystem error: cannot get file time: {} [",
                io_error_message(&error)
            )
            .into_bytes();
            message.extend(path_bytes(path));
            message.push(b']');
            RuntimeError::plain_diagnostic_bytes(message)
        })
}

fn values_equal(left: &Cell, right: &Cell) -> bool {
    match (&**left.borrow(), &**right.borrow()) {
        (Value::None, Value::None) => true,
        (Value::Bool(left), Value::Bool(right)) => left == right,
        (Value::Integer(left), Value::Integer(right)) => left == right,
        (Value::Float(left), Value::Float(right)) => left == right,
        (Value::String(left), Value::String(right)) => left == right,
        (Value::IrFunction(left), Value::IrFunction(right)) => Rc::ptr_eq(left, right),
        (Value::Class(left), Value::Class(right)) => Rc::ptr_eq(left, right),
        _ => values_identical(left, right),
    }
}

fn values_identical(left: &Cell, right: &Cell) -> bool {
    match (&**left.borrow(), &**right.borrow()) {
        (Value::None, Value::None) => true,
        (Value::Bool(left), Value::Bool(right)) => left == right,
        _ => Rc::ptr_eq(&left.borrow().identity, &right.borrow().identity),
    }
}

fn identity_id(value: &Cell) -> i64 {
    match &**value.borrow() {
        // Singleton identities retain a stable ordering.
        Value::Bool(true) => IDENTITY_BASE + 1,
        Value::None => IDENTITY_BASE + 2,
        Value::Bool(false) => IDENTITY_BASE + 3,
        _ => value.borrow().identity.0,
    }
}

fn class_member(class: &Class, name: &str) -> Option<Cell> {
    Env::find_local(&class.members, name)
}

fn inherit_class_members(class_environment: &Environment, base_classes: &[Rc<Class>]) -> Vec<Cell> {
    let mut constructors = Vec::new();
    for base in base_classes {
        let members = base.members.borrow().values.clone();
        let mut constructor = None;
        for (name, value) in members {
            if name == "__init__" {
                constructor = Some(copy_variable(value));
            } else {
                Env::define(class_environment, name, copy_variable(value));
            }
        }
        constructors.push(constructor.unwrap_or_else(|| cell(Value::Unbound(String::new()))));
    }
    constructors
}

fn legacy_float_string(value: f64) -> String {
    if value.is_nan() {
        if value.is_sign_negative() {
            "-nan".to_owned()
        } else {
            "nan".to_owned()
        }
    } else {
        format!("{value:.6}")
    }
}

#[cfg(unix)]
fn path_from_bytes(bytes: &[u8]) -> PathBuf {
    PathBuf::from(std::ffi::OsString::from_vec(bytes.to_vec()))
}

#[cfg(unix)]
fn filesystem_call_path(path: &Path) -> PathBuf {
    let bytes = path.as_os_str().as_bytes();
    let end = bytes
        .iter()
        .position(|byte| *byte == 0)
        .unwrap_or(bytes.len());
    PathBuf::from(std::ffi::OsString::from_vec(bytes[..end].to_vec()))
}

#[cfg(not(unix))]
fn filesystem_call_path(path: &Path) -> PathBuf {
    path.to_path_buf()
}

fn sidecar_path(path: &Path, suffix: &str) -> PathBuf {
    let mut sidecar = path.as_os_str().to_os_string();
    sidecar.push(suffix);
    PathBuf::from(sidecar)
}

#[cfg(not(unix))]
fn path_from_bytes(bytes: &[u8]) -> PathBuf {
    PathBuf::from(String::from_utf8_lossy(bytes).into_owned())
}

#[cfg(unix)]
fn path_bytes(path: &Path) -> Vec<u8> {
    path.as_os_str().as_bytes().to_vec()
}

#[cfg(not(unix))]
fn path_bytes(path: &Path) -> Vec<u8> {
    path.to_string_lossy().as_bytes().to_vec()
}

fn value_key(value: &Cell) -> Result<Vec<u8>, RuntimeError> {
    ensure_bound(value)?;
    Ok(match &**value.borrow() {
        Value::Integer(value) => format!("i{value}").into_bytes(),
        Value::Float(value) => format!("f{}", legacy_float_string(*value)).into_bytes(),
        Value::String(value) => {
            let mut key = Vec::with_capacity(value.len() + 1);
            key.push(b's');
            key.extend(value);
            key
        }
        Value::List(_) => {
            let mut key = vec![b'l'];
            key.extend(object_bytes(value)?);
            key
        }
        Value::Dict(_) => {
            let mut key = vec![b'd'];
            key.extend(object_bytes(value)?);
            key
        }
        _ => format!("p{}", identity_id(value)).into_bytes(),
    })
}

fn object_bytes(value: &Cell) -> Result<Vec<u8>, RuntimeError> {
    ensure_bound(value)?;
    let borrowed = value.borrow();
    Ok(match &**borrowed {
        Value::Unbound(_) => unreachable!("unbound values are rejected above"),
        Value::None => b"<no definition of to_str>".to_vec(),
        Value::Bool(value) => value.to_string().into_bytes(),
        Value::Integer(value) => value.to_string().into_bytes(),
        Value::Float(value) => legacy_float_string(*value).into_bytes(),
        Value::String(value) => value.clone(),
        Value::List(values) => {
            let values: Vec<_> = values
                .borrow()
                .iter()
                .map(|entry| entry.value.clone())
                .collect();
            drop(borrowed);
            let mut rendered = vec![b'['];
            for (index, value) in values.iter().enumerate() {
                if index != 0 {
                    rendered.extend(b", ");
                }
                rendered.extend(object_bytes(value)?);
            }
            rendered.push(b']');
            return Ok(rendered);
        }
        Value::Dict(values) => {
            let values = values.borrow().clone();
            drop(borrowed);
            if values.is_empty() {
                return Ok(b"{ }".to_vec());
            }
            let mut rendered = b"{ ".to_vec();
            for (index, entry) in values.iter().enumerate() {
                if index != 0 {
                    rendered.extend(b", ");
                }
                rendered.extend(object_bytes(&entry.key)?);
                rendered.extend(b" => ");
                rendered.extend(object_bytes(&entry.value)?);
            }
            rendered.extend(b" }");
            return Ok(rendered);
        }
        Value::IrFunction(_) => b"<function>".to_vec(),
        Value::Builtin(_) | Value::BoundMethod { .. } => b"<builtin-function>".to_vec(),
        Value::Path(path) => path_bytes(path),
        Value::File(_)
        | Value::ListIterator(_)
        | Value::Continuation(_)
        | Value::Namespace(_)
        | Value::Enum(_)
        | Value::Class(_)
        | Value::Instance { .. }
        | Value::UserIrMethod { .. }
        | Value::IrThunk(_) => b"<no definition of to_str>".to_vec(),
    })
}

fn dict_find_index(values: &[DictEntry], key: &Cell) -> Result<Option<usize>, RuntimeError> {
    let key = value_key(key)?;
    for (index, entry) in values.iter().enumerate() {
        if value_key(&entry.key)? == key {
            return Ok(Some(index));
        }
    }
    Ok(None)
}

fn dict_insert(values: &mut Vec<DictEntry>, key: Cell, value: Cell) -> Result<(), RuntimeError> {
    if let Some(index) = dict_find_index(values, &key)? {
        values[index].value = value;
    } else {
        let order_key = value_key(&key)?;
        let index = values.partition_point(|entry| entry.order_key <= order_key);
        values.insert(
            index,
            DictEntry {
                key,
                value,
                order_key,
            },
        );
    }
    Ok(())
}

fn dict_index(values: &mut Vec<DictEntry>, key: Cell) -> Result<Cell, RuntimeError> {
    if let Some(index) = dict_find_index(values, &key)? {
        return Ok(values[index].value.clone());
    }
    let value = cell(Value::Unbound(String::new()));
    let key = copy_variable(key);
    let order_key = value_key(&key)?;
    let index = values.partition_point(|entry| entry.order_key <= order_key);
    values.insert(
        index,
        DictEntry {
            key,
            value: value.clone(),
            order_key,
        },
    );
    Ok(value)
}

fn repl_if_has_final_else(statement: Option<&Stmt>) -> bool {
    match statement {
        Some(Stmt::Block(_)) => true,
        Some(Stmt::If { else_branch, .. }) => repl_if_has_final_else(else_branch.as_deref()),
        _ => false,
    }
}

fn repl_block_declaration_is_complete(source: &str, declaration: &crate::ast::Declaration) -> bool {
    if !source.trim_end().ends_with('}') {
        return false;
    }
    let [
        Binding::Name {
            by_reference: true, ..
        },
    ] = declaration.bindings.as_slice()
    else {
        return false;
    };
    match declaration.values.as_slice() {
        [Expr::Lambda { .. }] => !source.trim_start().starts_with("@&"),
        [Expr::Class { .. }] => source.trim_start().starts_with("class"),
        _ => false,
    }
}

impl From<std::io::Error> for RuntimeError {
    fn from(error: std::io::Error) -> Self {
        Self::new(error.to_string())
    }
}

fn clone_class_members(class: &Class) -> Environment {
    let fields = Env::root();
    for (name, value) in class.members.borrow().values.clone() {
        Env::define(&fields, name, copy_variable(value));
    }
    fields
}
