use std::cell::RefCell;
use std::collections::{HashMap, HashSet, VecDeque};
use std::fmt;
use std::fs::{self, File, OpenOptions};
use std::io::{BufRead, BufReader, Read, Seek, SeekFrom, Write};
use std::ops::{Deref, DerefMut};
use std::path::{Path, PathBuf};
use std::rc::Rc;
use std::time::{SystemTime, UNIX_EPOCH};

use crate::ast::{
    Argument, Binding, Block, Declaration, Expr, Literal, ModulePart, Parameter, Stmt,
};
use crate::{Location, ParseError, Parser};

type Cell = Rc<RefCell<ValueSlot>>;
type Environment = Rc<RefCell<Env>>;
type Continuation = Rc<dyn Fn(&mut Interpreter, Cell) -> Result<(), RuntimeError>>;
type Step = Rc<dyn Fn(&mut Interpreter) -> Result<(), RuntimeError>>;
type ReturnContinuation = Rc<dyn Fn(&mut Interpreter, Vec<Cell>) -> Result<(), RuntimeError>>;

#[derive(Clone)]
struct Control {
    on_return: ReturnContinuation,
    on_break: Option<Step>,
    on_continue: Option<Step>,
}

#[derive(Clone)]
struct ValueSlot {
    value: Value,
    identity: Rc<()>,
}

impl ValueSlot {
    fn new(value: Value) -> Self {
        Self {
            value,
            identity: Rc::new(()),
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
struct Function {
    parameters: Vec<Parameter>,
    body: Block,
    closure: Environment,
}

#[derive(Clone)]
struct Thunk {
    expression: Expr,
    environment: Environment,
    cached: Rc<RefCell<Option<Cell>>>,
}

#[derive(Clone)]
struct Class {
    members: Environment,
    bases: Vec<Rc<Class>>,
}

struct RuntimeFile {
    file: Option<File>,
    eof: bool,
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
    None,
    Bool(bool),
    Integer(i64),
    Float(f64),
    String(String),
    List(Rc<RefCell<Vec<Cell>>>),
    Dict(Rc<RefCell<HashMap<String, Cell>>>),
    File(Rc<RefCell<RuntimeFile>>),
    Path(PathBuf),
    Continuation(Continuation),
    Function(Rc<Function>),
    Namespace(Environment),
    Class(Rc<Class>),
    Instance {
        class: Rc<Class>,
        fields: Environment,
    },
    UserMethod {
        function: Rc<Function>,
        receiver: Cell,
    },
    Builtin(Builtin),
    BoundMethod {
        receiver: Cell,
        name: String,
    },
    Thunk(Thunk),
    Multi(Vec<Cell>),
}

struct Env {
    values: HashMap<String, Cell>,
    parent: Option<Environment>,
}

impl Env {
    fn root() -> Environment {
        Rc::new(RefCell::new(Self {
            values: HashMap::new(),
            parent: None,
        }))
    }

    fn child(parent: &Environment) -> Environment {
        Rc::new(RefCell::new(Self {
            values: HashMap::new(),
            parent: Some(Rc::clone(parent)),
        }))
    }

    fn find(environment: &Environment, name: &str) -> Option<Cell> {
        let (value, parent) = {
            let borrowed = environment.borrow();
            (borrowed.values.get(name).cloned(), borrowed.parent.clone())
        };
        value.or_else(|| parent.and_then(|parent| Self::find(&parent, name)))
    }

    fn define(environment: &Environment, name: String, value: Cell) {
        environment.borrow_mut().values.insert(name, value);
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct RuntimeError {
    pub location: Option<Location>,
    pub message: String,
}

impl RuntimeError {
    fn new(message: impl Into<String>) -> Self {
        Self {
            location: None,
            message: message.into(),
        }
    }

    fn at(location: Location, message: impl Into<String>) -> Self {
        Self {
            location: Some(location),
            message: message.into(),
        }
    }
}

impl fmt::Display for RuntimeError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if let Some(location) = self.location {
            write!(
                f,
                "{}:{}: error: {}",
                location.line, location.column, self.message
            )
        } else {
            write!(f, "error: {}", self.message)
        }
    }
}

impl std::error::Error for RuntimeError {}

impl From<ParseError> for RuntimeError {
    fn from(error: ParseError) -> Self {
        Self::at(error.location, error.message)
    }
}

enum Flow {
    Next,
    Return(Vec<Cell>),
    Break,
    Continue,
}

pub struct Interpreter {
    globals: Environment,
    output: String,
    arguments: Vec<String>,
    directories: Vec<PathBuf>,
    modules: HashMap<PathBuf, Cell>,
    pending_steps: VecDeque<Step>,
    halted: bool,
    prefix_operators: HashSet<String>,
    infix_operators: HashMap<String, u16>,
}

impl Default for Interpreter {
    fn default() -> Self {
        Self::new()
    }
}

impl Interpreter {
    #[must_use]
    pub fn new() -> Self {
        let globals = Env::root();
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
            Env::define(&globals, name.to_owned(), cell(Value::Builtin(builtin)));
        }
        Self {
            globals,
            output: String::new(),
            arguments: Vec::new(),
            directories: vec![std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."))],
            modules: HashMap::new(),
            pending_steps: VecDeque::new(),
            halted: false,
            prefix_operators: HashSet::new(),
            infix_operators: HashMap::new(),
        }
    }

    #[must_use]
    pub fn with_arguments(arguments: Vec<String>) -> Self {
        let mut interpreter = Self::new();
        interpreter.arguments = arguments;
        interpreter
    }

    pub fn run(&mut self, source: &str, name: &str) -> Result<String, RuntimeError> {
        self.run_internal(source, name, false)
    }

    pub fn run_repl(&mut self, source: &str) -> Result<String, RuntimeError> {
        self.run_internal(source, "<stdin>", true)
    }

    #[must_use]
    pub fn is_halted(&self) -> bool {
        self.halted
    }

    fn run_internal(
        &mut self,
        source: &str,
        name: &str,
        print_expressions: bool,
    ) -> Result<String, RuntimeError> {
        let path = Path::new(name);
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
        let mut parser = Parser::new(source, name)?;
        let environment = Rc::clone(&self.globals);
        let finish: Step = Rc::new(|_| Ok(()));
        let top_return: ReturnContinuation = Rc::new(|interpreter, _| {
            interpreter.halted = true;
            Ok(())
        });
        let control = Control {
            on_return: top_return,
            on_break: None,
            on_continue: None,
        };
        let result = (|| {
            loop {
                for operator in &self.prefix_operators {
                    parser.add_prefix_operator(operator.clone());
                }
                for (operator, precedence) in &self.infix_operators {
                    parser.add_infix_operator(operator.clone(), *precedence);
                }
                let Some(mut statement) = parser.parse_next()? else {
                    break;
                };
                if print_expressions && let Stmt::Expression(expression) = statement {
                    statement = Stmt::Expression(Expr::Call {
                        callee: Box::new(Expr::Identifier(
                            "println".to_owned(),
                            Location { line: 1, column: 0 },
                        )),
                        arguments: vec![Argument {
                            value: expression,
                            unpack: false,
                        }],
                        location: Location { line: 1, column: 0 },
                    });
                }
                self.execute_sequence_cps(
                    vec![statement],
                    0,
                    environment.clone(),
                    control.clone(),
                    finish.clone(),
                )?;
                while !self.halted
                    && let Some(step) = self.pending_steps.pop_front()
                {
                    step(self)?;
                }
                if self.halted {
                    break;
                }
            }
            Ok(std::mem::take(&mut self.output))
        })();
        if pushed_directory {
            self.directories.pop();
        }
        result
    }

    fn execute_sequence_cps(
        &mut self,
        statements: Block,
        index: usize,
        environment: Environment,
        control: Control,
        done: Step,
    ) -> Result<(), RuntimeError> {
        let Some(statement) = statements.get(index).cloned() else {
            return self.advance(done);
        };
        let next_statements = statements.clone();
        let next_environment = environment.clone();
        let next_control = control.clone();
        let next_done = done.clone();
        let next: Step = Rc::new(move |interpreter| {
            interpreter.execute_sequence_cps(
                next_statements.clone(),
                index + 1,
                next_environment.clone(),
                next_control.clone(),
                next_done.clone(),
            )
        });
        self.execute_statement_cps(statement, environment, control, next)
    }

    fn execute_statement_cps(
        &mut self,
        statement: Stmt,
        environment: Environment,
        control: Control,
        next: Step,
    ) -> Result<(), RuntimeError> {
        match statement {
            Stmt::Expression(expression) => {
                let continuation: Continuation =
                    Rc::new(move |interpreter, _| interpreter.advance(next.clone()));
                self.evaluate_cps(expression, environment, continuation)
            }
            Stmt::Declaration(declaration) => {
                let values = declaration.values.clone();
                let target_environment = environment.clone();
                let continuation = Rc::new(move |interpreter: &mut Interpreter, values| {
                    interpreter.declare_evaluated(&declaration, values, &target_environment)?;
                    interpreter.advance(next.clone())
                });
                self.evaluate_many_cps(values, 0, environment, Vec::new(), continuation)
            }
            Stmt::If {
                condition,
                then_block,
                else_branch,
            } => {
                let branch_environment = environment.clone();
                let branch_control = control.clone();
                let continuation: Continuation = Rc::new(move |interpreter, condition| {
                    if interpreter.truthy(&condition)? {
                        interpreter.execute_sequence_cps(
                            then_block.clone(),
                            0,
                            Env::child(&branch_environment),
                            branch_control.clone(),
                            next.clone(),
                        )
                    } else if let Some(branch) = &else_branch {
                        interpreter.execute_statement_cps(
                            (**branch).clone(),
                            branch_environment.clone(),
                            branch_control.clone(),
                            next.clone(),
                        )
                    } else {
                        interpreter.advance(next.clone())
                    }
                });
                self.evaluate_cps(condition, environment, continuation)
            }
            Stmt::Block(block) => {
                self.execute_sequence_cps(block, 0, Env::child(&environment), control, next)
            }
            Stmt::While { condition, body } => {
                self.execute_while_cps(condition, body, environment, control, next)
            }
            Stmt::DoWhile { body, condition } => {
                let loop_environment = environment.clone();
                let loop_control = control.clone();
                let loop_next = next.clone();
                let repeated_body = body.clone();
                let after_body: Step = Rc::new(move |interpreter| {
                    let condition = condition.clone();
                    let body = repeated_body.clone();
                    let environment = loop_environment.clone();
                    let control = loop_control.clone();
                    let next = loop_next.clone();
                    interpreter.schedule(Rc::new(move |interpreter| {
                        interpreter.execute_while_cps(
                            condition.clone(),
                            body.clone(),
                            environment.clone(),
                            control.clone(),
                            next.clone(),
                        )
                    }));
                    Ok(())
                });
                let body_control = Control {
                    on_return: control.on_return,
                    on_break: Some(next),
                    on_continue: Some(after_body.clone()),
                };
                self.execute_sequence_cps(
                    body,
                    0,
                    Env::child(&environment),
                    body_control,
                    after_body,
                )
            }
            Stmt::Foreach {
                iterable,
                binding,
                body,
            } => {
                let loop_environment = environment.clone();
                let loop_control = control.clone();
                let continuation: Continuation = Rc::new(move |interpreter, iterable| {
                    let iterable = interpreter.force(iterable)?;
                    let list = match &**iterable.borrow() {
                        Value::List(items) => Some(items.clone()),
                        _ => None,
                    };
                    if let Some(items) = list {
                        interpreter.execute_list_foreach_cps(
                            items,
                            0,
                            binding.clone(),
                            body.clone(),
                            loop_environment.clone(),
                            loop_control.clone(),
                            next.clone(),
                        )
                    } else {
                        let location = Location { line: 0, column: 0 };
                        let iterator = interpreter.member(iterable, "__iterator__", location)?;
                        let binding = binding.clone();
                        let body = body.clone();
                        let environment = loop_environment.clone();
                        let control = loop_control.clone();
                        let next = next.clone();
                        let on_iterator: Continuation = Rc::new(move |interpreter, iterator| {
                            interpreter.execute_iterator_foreach_cps(
                                iterator,
                                binding.clone(),
                                body.clone(),
                                environment.clone(),
                                control.clone(),
                                next.clone(),
                            )
                        });
                        interpreter.invoke_cps(
                            iterator,
                            Vec::new(),
                            location,
                            loop_environment.clone(),
                            on_iterator,
                        )
                    }
                });
                self.evaluate_cps(iterable, environment, continuation)
            }
            Stmt::Return(expressions) => {
                let on_return = control.on_return;
                let continuation = Rc::new(move |interpreter: &mut Interpreter, values| {
                    interpreter.resume_return(on_return.clone(), flatten_values(values))
                });
                self.evaluate_many_cps(expressions, 0, environment, Vec::new(), continuation)
            }
            Stmt::Break => {
                control
                    .on_break
                    .ok_or_else(|| RuntimeError::new("break outside loop"))?(self)
            }
            Stmt::Continue => {
                control
                    .on_continue
                    .ok_or_else(|| RuntimeError::new("continue outside loop"))?(self)
            }
            other
            @ (Stmt::Import { .. } | Stmt::PrefixOperator(_) | Stmt::InfixOperator { .. }) => {
                match self.execute(&other, &environment)? {
                    Flow::Next => self.advance(next),
                    Flow::Return(values) => (control.on_return)(self, values),
                    Flow::Break => control
                        .on_break
                        .ok_or_else(|| RuntimeError::new("break outside loop"))?(
                        self
                    ),
                    Flow::Continue => control
                        .on_continue
                        .ok_or_else(|| RuntimeError::new("continue outside loop"))?(
                        self
                    ),
                }
            }
        }
    }

    fn execute_while_cps(
        &mut self,
        condition: Expr,
        body: Block,
        environment: Environment,
        control: Control,
        next: Step,
    ) -> Result<(), RuntimeError> {
        let loop_condition = condition.clone();
        let loop_body = body.clone();
        let loop_environment = environment.clone();
        let loop_control = control.clone();
        let loop_next = next.clone();
        let after_iteration: Step = Rc::new(move |interpreter| {
            let condition = loop_condition.clone();
            let body = loop_body.clone();
            let environment = loop_environment.clone();
            let control = loop_control.clone();
            let next = loop_next.clone();
            interpreter.schedule(Rc::new(move |interpreter| {
                interpreter.execute_while_cps(
                    condition.clone(),
                    body.clone(),
                    environment.clone(),
                    control.clone(),
                    next.clone(),
                )
            }));
            Ok(())
        });
        let condition_environment = environment.clone();
        let condition_control = control.clone();
        let continuation: Continuation = Rc::new(move |interpreter, value| {
            if interpreter.truthy(&value)? {
                let body_control = Control {
                    on_return: condition_control.on_return.clone(),
                    on_break: Some(next.clone()),
                    on_continue: Some(after_iteration.clone()),
                };
                interpreter.execute_sequence_cps(
                    body.clone(),
                    0,
                    Env::child(&condition_environment),
                    body_control,
                    after_iteration.clone(),
                )
            } else {
                interpreter.advance(next.clone())
            }
        });
        self.evaluate_cps(condition, environment, continuation)
    }

    #[allow(clippy::too_many_arguments)]
    fn execute_list_foreach_cps(
        &mut self,
        items: Rc<RefCell<Vec<Cell>>>,
        index: usize,
        binding: Option<String>,
        body: Block,
        environment: Environment,
        control: Control,
        next: Step,
    ) -> Result<(), RuntimeError> {
        let Some(item) = items.borrow().get(index).cloned() else {
            return self.advance(next);
        };
        let repeated_items = items.clone();
        let repeated_binding = binding.clone();
        let repeated_body = body.clone();
        let repeated_environment = environment.clone();
        let repeated_control = control.clone();
        let repeated_next = next.clone();
        let after_iteration: Step = Rc::new(move |interpreter| {
            interpreter.execute_list_foreach_cps(
                repeated_items.clone(),
                index + 1,
                repeated_binding.clone(),
                repeated_body.clone(),
                repeated_environment.clone(),
                repeated_control.clone(),
                repeated_next.clone(),
            )
        });
        let iteration_environment = Env::child(&environment);
        if let Some(name) = binding {
            Env::define(&iteration_environment, name, item);
        }
        let body_control = Control {
            on_return: control.on_return,
            on_break: Some(next),
            on_continue: Some(after_iteration.clone()),
        };
        self.execute_sequence_cps(
            body,
            0,
            iteration_environment,
            body_control,
            after_iteration,
        )
    }

    fn execute_iterator_foreach_cps(
        &mut self,
        iterator: Cell,
        binding: Option<String>,
        body: Block,
        environment: Environment,
        control: Control,
        next: Step,
    ) -> Result<(), RuntimeError> {
        let location = Location { line: 0, column: 0 };
        let has_next = self.member(iterator.clone(), "__has_next__", location)?;
        let invocation_environment = environment.clone();
        let continuation: Continuation = Rc::new(move |interpreter, has_next| {
            if !interpreter.truthy(&has_next)? {
                return interpreter.advance(next.clone());
            }
            let next_method = interpreter.member(iterator.clone(), "__next__", location)?;
            let repeated_iterator = iterator.clone();
            let repeated_binding = binding.clone();
            let repeated_body = body.clone();
            let repeated_environment = environment.clone();
            let repeated_control = control.clone();
            let repeated_next = next.clone();
            let on_item: Continuation = Rc::new(move |interpreter, item| {
                let iterator = repeated_iterator.clone();
                let binding = repeated_binding.clone();
                let body = repeated_body.clone();
                let environment = repeated_environment.clone();
                let control = repeated_control.clone();
                let next = repeated_next.clone();
                let after_iteration: Step = Rc::new(move |interpreter| {
                    interpreter.execute_iterator_foreach_cps(
                        iterator.clone(),
                        binding.clone(),
                        body.clone(),
                        environment.clone(),
                        control.clone(),
                        next.clone(),
                    )
                });
                let iteration_environment = Env::child(&repeated_environment);
                if let Some(name) = &repeated_binding {
                    Env::define(&iteration_environment, name.clone(), item.clone());
                }
                let body_control = Control {
                    on_return: repeated_control.on_return.clone(),
                    on_break: Some(repeated_next.clone()),
                    on_continue: Some(after_iteration.clone()),
                };
                interpreter.execute_sequence_cps(
                    repeated_body.clone(),
                    0,
                    iteration_environment,
                    body_control,
                    after_iteration,
                )
            });
            interpreter.invoke_cps(
                next_method,
                Vec::new(),
                location,
                environment.clone(),
                on_item,
            )
        });
        self.invoke_cps(
            has_next,
            Vec::new(),
            location,
            invocation_environment,
            continuation,
        )
    }

    fn evaluate_cps(
        &mut self,
        expression: Expr,
        environment: Environment,
        continuation: Continuation,
    ) -> Result<(), RuntimeError> {
        match expression {
            Expr::Unary {
                operator,
                operand,
                location,
            } => {
                let operation_environment = environment.clone();
                let next: Continuation = Rc::new(move |interpreter, operand| {
                    if matches!(operator.as_str(), "not" | "!" | "-" | "~") {
                        let result = interpreter.unary(
                            &operator,
                            operand,
                            location,
                            &operation_environment,
                        )?;
                        interpreter.resume(continuation.clone(), result)
                    } else {
                        let function =
                            Env::find(&operation_environment, &operator).ok_or_else(|| {
                                RuntimeError::at(location, format!("no prefix operator {operator}"))
                            })?;
                        interpreter.invoke_cps(
                            function,
                            vec![operand],
                            location,
                            operation_environment.clone(),
                            continuation.clone(),
                        )
                    }
                });
                self.evaluate_cps(*operand, environment, next)
            }
            Expr::Binary {
                left,
                operator,
                right,
                location: _,
            } if operator == ":" => {
                let assignment_environment = environment.clone();
                let target_continuation = continuation.clone();
                let right_expression = *right;
                let on_target: Continuation = Rc::new(move |interpreter, target| {
                    let target = interpreter.reference(target)?;
                    let assigned_target = target.clone();
                    let assignment_continuation = target_continuation.clone();
                    let on_value: Continuation = Rc::new(move |interpreter, value| {
                        let value = interpreter.force(value)?;
                        *assigned_target.borrow_mut() = value.borrow().clone();
                        interpreter.resume(assignment_continuation.clone(), assigned_target.clone())
                    });
                    interpreter.evaluate_cps(
                        right_expression.clone(),
                        assignment_environment.clone(),
                        on_value,
                    )
                });
                self.evaluate_lvalue_cps(*left, environment, true, on_target)
            }
            Expr::Binary {
                left,
                operator,
                right,
                location,
            } => {
                let right_environment = environment.clone();
                let operation_environment = environment.clone();
                let right_expression = *right;
                let on_left: Continuation = Rc::new(move |interpreter, left| {
                    if operator == "and" && !interpreter.truthy(&left)? {
                        return interpreter.resume(continuation.clone(), cell(Value::Bool(false)));
                    }
                    if operator == "or" && interpreter.truthy(&left)? {
                        return interpreter.resume(continuation.clone(), cell(Value::Bool(true)));
                    }
                    let operation = operator.clone();
                    let left_value = left.clone();
                    let final_continuation = continuation.clone();
                    let operation_environment = operation_environment.clone();
                    let on_right: Continuation = Rc::new(move |interpreter, right| {
                        if is_builtin_binary_operator(&operation) {
                            let result = interpreter.binary(
                                &operation,
                                left_value.clone(),
                                right,
                                location,
                                &operation_environment,
                            )?;
                            interpreter.resume(final_continuation.clone(), result)
                        } else {
                            let function = Env::find(&operation_environment, &operation)
                                .ok_or_else(|| {
                                    RuntimeError::at(
                                        location,
                                        format!("no infix operator {operation}"),
                                    )
                                })?;
                            interpreter.invoke_cps(
                                function,
                                vec![left_value.clone(), right],
                                location,
                                operation_environment.clone(),
                                final_continuation.clone(),
                            )
                        }
                    });
                    interpreter.evaluate_cps(
                        right_expression.clone(),
                        right_environment.clone(),
                        on_right,
                    )
                });
                self.evaluate_cps(*left, environment, on_left)
            }
            Expr::Call {
                callee,
                arguments,
                location,
            } => {
                let call_environment = environment.clone();
                let on_callee: Continuation = Rc::new(move |interpreter, callee| {
                    let invoke_environment = call_environment.clone();
                    let invoke_continuation = continuation.clone();
                    let on_arguments =
                        Rc::new(move |interpreter: &mut Interpreter, arguments: Vec<Cell>| {
                            interpreter.invoke_cps(
                                callee.clone(),
                                arguments,
                                location,
                                invoke_environment.clone(),
                                invoke_continuation.clone(),
                            )
                        });
                    interpreter.evaluate_arguments_cps(
                        arguments.clone(),
                        0,
                        call_environment.clone(),
                        Vec::new(),
                        on_arguments,
                    )
                });
                self.evaluate_cps(*callee, environment, on_callee)
            }
            Expr::Member {
                object,
                name,
                location,
            } => {
                let next: Continuation = Rc::new(move |interpreter, object| {
                    let member = interpreter.member(object, &name, location)?;
                    interpreter.resume(continuation.clone(), member)
                });
                self.evaluate_cps(*object, environment, next)
            }
            Expr::Index {
                object,
                index,
                location,
            } => {
                let index_environment = environment.clone();
                let index_expression = *index;
                let on_object: Continuation = Rc::new(move |interpreter, object| {
                    let indexed_object = object.clone();
                    let final_continuation = continuation.clone();
                    let on_index: Continuation = Rc::new(move |interpreter, index| {
                        let result =
                            interpreter.index_values(indexed_object.clone(), index, location)?;
                        interpreter.resume(final_continuation.clone(), result)
                    });
                    interpreter.evaluate_cps(
                        index_expression.clone(),
                        index_environment.clone(),
                        on_index,
                    )
                });
                self.evaluate_cps(*object, environment, on_object)
            }
            Expr::Conditional {
                condition,
                then_value,
                else_value,
                ..
            } => {
                let branch_environment = environment.clone();
                let on_condition: Continuation = Rc::new(move |interpreter, condition| {
                    let branch = if interpreter.truthy(&condition)? {
                        (*then_value).clone()
                    } else {
                        (*else_value).clone()
                    };
                    interpreter.evaluate_cps(
                        branch,
                        branch_environment.clone(),
                        continuation.clone(),
                    )
                });
                self.evaluate_cps(*condition, environment, on_condition)
            }
            Expr::List(expressions) => {
                let final_continuation = continuation.clone();
                let on_values = Rc::new(move |interpreter: &mut Interpreter, values: Vec<Cell>| {
                    let values = values.into_iter().map(copy_variable).collect();
                    interpreter.resume(final_continuation.clone(), cell(list_value(values)))
                });
                self.evaluate_many_cps(expressions, 0, environment, Vec::new(), on_values)
            }
            simple => {
                let value = self.evaluate(&simple, &environment)?;
                self.resume(continuation, value)
            }
        }
    }

    fn evaluate_lvalue_cps(
        &mut self,
        expression: Expr,
        environment: Environment,
        create: bool,
        continuation: Continuation,
    ) -> Result<(), RuntimeError> {
        match expression {
            Expr::Identifier(name, location) => {
                let value = if let Some(value) = Env::find(&environment, &name) {
                    value
                } else if create {
                    let value = cell(Value::None);
                    Env::define(&environment, name, value.clone());
                    value
                } else {
                    return Err(RuntimeError::at(
                        location,
                        format!("no variable named {name}"),
                    ));
                };
                self.resume(continuation, value)
            }
            Expr::Member {
                object,
                name,
                location,
            } => {
                let next: Continuation = Rc::new(move |interpreter, object| {
                    let value = interpreter.member_lvalue(object, &name, location, create)?;
                    interpreter.resume(continuation.clone(), value)
                });
                self.evaluate_cps(*object, environment, next)
            }
            Expr::Index {
                object,
                index,
                location,
            } => {
                let index_environment = environment.clone();
                let index_expression = *index;
                let on_object: Continuation = Rc::new(move |interpreter, object| {
                    let object = object.clone();
                    let final_continuation = continuation.clone();
                    let on_index: Continuation = Rc::new(move |interpreter, index| {
                        let value = interpreter.index_values(object.clone(), index, location)?;
                        interpreter.resume(final_continuation.clone(), value)
                    });
                    interpreter.evaluate_cps(
                        index_expression.clone(),
                        index_environment.clone(),
                        on_index,
                    )
                });
                self.evaluate_cps(*object, environment, on_object)
            }
            _ => Err(RuntimeError::new("expression is not assignable")),
        }
    }

    fn evaluate_many_cps(
        &mut self,
        expressions: Vec<Expr>,
        index: usize,
        environment: Environment,
        values: Vec<Cell>,
        continuation: ReturnContinuation,
    ) -> Result<(), RuntimeError> {
        let Some(expression) = expressions.get(index).cloned() else {
            return self.resume_return(continuation, values);
        };
        let next_expressions = expressions.clone();
        let next_environment = environment.clone();
        let next_continuation = continuation.clone();
        let next: Continuation = Rc::new(move |interpreter, value| {
            let mut values = values.clone();
            values.push(value);
            interpreter.evaluate_many_cps(
                next_expressions.clone(),
                index + 1,
                next_environment.clone(),
                values,
                next_continuation.clone(),
            )
        });
        self.evaluate_cps(expression, environment, next)
    }

    fn evaluate_arguments_cps(
        &mut self,
        arguments: Vec<Argument>,
        index: usize,
        environment: Environment,
        values: Vec<Cell>,
        continuation: ReturnContinuation,
    ) -> Result<(), RuntimeError> {
        let Some(argument) = arguments.get(index).cloned() else {
            return self.resume_return(continuation, values);
        };
        let next_arguments = arguments.clone();
        let next_environment = environment.clone();
        let next_continuation = continuation.clone();
        let next: Continuation = Rc::new(move |interpreter, value| {
            let mut values = values.clone();
            if argument.unpack {
                let value = interpreter.force(value)?;
                match &**value.borrow() {
                    Value::List(items) => values.extend(items.borrow().iter().cloned()),
                    Value::Multi(items) => values.extend(items.iter().cloned()),
                    _ => return Err(RuntimeError::new("cannot unpack non-list argument")),
                }
            } else {
                values.push(value);
            }
            interpreter.evaluate_arguments_cps(
                next_arguments.clone(),
                index + 1,
                next_environment.clone(),
                values,
                next_continuation.clone(),
            )
        });
        self.evaluate_cps(argument.value, environment, next)
    }

    fn invoke_cps(
        &mut self,
        callee: Cell,
        arguments: Vec<Cell>,
        location: Location,
        environment: Environment,
        continuation: Continuation,
    ) -> Result<(), RuntimeError> {
        let callee = self.force(callee)?;
        let value = callee.borrow().value.clone();
        match value {
            Value::Function(function) => {
                self.call_function_cps(function, arguments, location, continuation)
            }
            Value::UserMethod { function, receiver } => {
                let mut arguments = arguments;
                arguments.insert(0, receiver);
                self.call_function_cps(function, arguments, location, continuation)
            }
            Value::Continuation(captured) => {
                let value = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                self.resume(captured, value)
            }
            Value::Builtin(Builtin::CallWithCurrentContinuation) => {
                let function = arguments
                    .first()
                    .cloned()
                    .ok_or_else(|| RuntimeError::at(location, "call/cc expects one argument"))?;
                self.invoke_cps(
                    function,
                    vec![cell(Value::Continuation(continuation.clone()))],
                    location,
                    environment,
                    continuation,
                )
            }
            Value::Builtin(Builtin::Exit) => {
                self.halted = true;
                self.pending_steps.clear();
                Ok(())
            }
            Value::Builtin(builtin) => {
                let value = self.call_builtin(builtin, arguments, &environment)?;
                self.resume(continuation, value)
            }
            Value::BoundMethod { receiver, name } => {
                let value = self.call_method(receiver, &name, arguments)?;
                self.resume(continuation, value)
            }
            Value::Class(class) => self.instantiate_cps(class, arguments, location, continuation),
            _ => Err(RuntimeError::at(location, "object is not callable")),
        }
    }

    fn call_function_cps(
        &mut self,
        function: Rc<Function>,
        arguments: Vec<Cell>,
        location: Location,
        continuation: Continuation,
    ) -> Result<(), RuntimeError> {
        let call_environment = self.bind_function_arguments(&function, arguments, location)?;
        let normal_continuation = continuation.clone();
        let done: Step = Rc::new(move |interpreter| {
            interpreter.resume(normal_continuation.clone(), cell(Value::None))
        });
        let return_continuation = continuation.clone();
        let on_return: ReturnContinuation = Rc::new(move |interpreter, values| {
            let value = match values.len() {
                0 => cell(Value::None),
                1 => values[0].clone(),
                _ => cell(Value::Multi(values)),
            };
            interpreter.resume(return_continuation.clone(), value)
        });
        let control = Control {
            on_return,
            on_break: None,
            on_continue: None,
        };
        self.execute_sequence_cps(function.body.clone(), 0, call_environment, control, done)
    }

    fn instantiate_cps(
        &mut self,
        class: Rc<Class>,
        arguments: Vec<Cell>,
        location: Location,
        continuation: Continuation,
    ) -> Result<(), RuntimeError> {
        let instance = cell(Value::Instance {
            class: class.clone(),
            fields: clone_class_members(&class),
        });
        if let Some(constructor) = class_member(&class, "__init__") {
            let function = constructor.borrow().value.clone();
            let Value::Function(function) = function else {
                return Err(RuntimeError::at(location, "__init__ must be callable"));
            };
            let mut constructor_arguments = arguments;
            constructor_arguments.insert(0, instance.clone());
            let on_constructed: Continuation = Rc::new(move |interpreter, _| {
                interpreter.resume(continuation.clone(), instance.clone())
            });
            self.call_function_cps(function, constructor_arguments, location, on_constructed)
        } else if arguments.is_empty() {
            self.resume(continuation, instance)
        } else {
            Err(RuntimeError::at(location, "too many arguments"))
        }
    }

    fn bind_function_arguments(
        &mut self,
        function: &Function,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<Environment, RuntimeError> {
        let call_environment = Env::child(&function.closure);
        let mut argument_index = 0;
        for parameter in &function.parameters {
            if parameter.variadic {
                let mut items = Vec::new();
                for argument in &arguments[argument_index..] {
                    if parameter.by_reference {
                        items.push(self.reference(argument.clone())?);
                    } else {
                        items.push(copy_variable(argument.clone()));
                    }
                }
                Env::define(
                    &call_environment,
                    parameter.name.clone(),
                    cell(list_value(items)),
                );
                argument_index = arguments.len();
                continue;
            }
            let argument = if let Some(argument) = arguments.get(argument_index) {
                argument_index += 1;
                argument.clone()
            } else if let Some(default) = &parameter.default {
                self.evaluate(default, &function.closure)?
            } else {
                return Err(RuntimeError::at(location, "not enough arguments"));
            };
            let value = if parameter.by_reference {
                self.reference(argument)?
            } else {
                copy_variable(argument)
            };
            Env::define(&call_environment, parameter.name.clone(), value);
        }
        if argument_index < arguments.len() {
            return Err(RuntimeError::at(location, "too many arguments"));
        }
        Ok(call_environment)
    }

    fn declare_evaluated(
        &mut self,
        declaration: &Declaration,
        mut values: Vec<Cell>,
        environment: &Environment,
    ) -> Result<(), RuntimeError> {
        if values.len() == 1 && declaration.bindings.len() > 1 {
            let value = self.force(values[0].clone())?;
            match &**value.borrow() {
                Value::List(items) => values = items.borrow().clone(),
                Value::Multi(items) => values = items.clone(),
                _ => {}
            }
        }
        while values.len() < declaration.bindings.len() {
            values.push(cell(Value::None));
        }
        for (index, binding) in declaration.bindings.iter().enumerate() {
            self.bind(
                binding,
                values
                    .get(index)
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None)),
                environment,
            )?;
        }
        Ok(())
    }

    fn index_values(
        &mut self,
        object: Cell,
        index: Cell,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        let object = self.force(object)?;
        let index = self.force(index)?;
        let index_value = index.borrow().value.clone();
        match (&mut **object.borrow_mut(), index_value) {
            (Value::List(items), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                items
                    .borrow()
                    .get(index)
                    .cloned()
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::String(value), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                value
                    .chars()
                    .nth(index)
                    .map(|character| cell(Value::String(character.to_string())))
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::Dict(values), key) => {
                let key = value_key(&key)?;
                Ok(values
                    .borrow_mut()
                    .entry(key)
                    .or_insert_with(|| cell(Value::None))
                    .clone())
            }
            (_, Value::Integer(_)) => Err(RuntimeError::at(location, "object is not indexable")),
            _ => Err(RuntimeError::at(location, "index should be integer")),
        }
    }

    fn execute_block(
        &mut self,
        block: &Block,
        environment: &Environment,
    ) -> Result<Flow, RuntimeError> {
        for statement in block {
            let flow = self.execute(statement, environment)?;
            if !matches!(flow, Flow::Next) {
                return Ok(flow);
            }
        }
        Ok(Flow::Next)
    }

    fn execute(
        &mut self,
        statement: &Stmt,
        environment: &Environment,
    ) -> Result<Flow, RuntimeError> {
        match statement {
            Stmt::Expression(expression) => {
                self.evaluate(expression, environment)?;
                Ok(Flow::Next)
            }
            Stmt::Declaration(declaration) => {
                self.declare(declaration, environment)?;
                Ok(Flow::Next)
            }
            Stmt::PrefixOperator(operator) => {
                self.prefix_operators.insert(operator.clone());
                Ok(Flow::Next)
            }
            Stmt::InfixOperator {
                operator,
                precedence,
            } => {
                self.infix_operators.insert(operator.clone(), *precedence);
                Ok(Flow::Next)
            }
            Stmt::If {
                condition,
                then_block,
                else_branch,
            } => {
                let condition = self.evaluate(condition, environment)?;
                if self.truthy(&condition)? {
                    self.execute_block(then_block, &Env::child(environment))
                } else if let Some(branch) = else_branch {
                    self.execute(branch, environment)
                } else {
                    Ok(Flow::Next)
                }
            }
            Stmt::Block(block) => self.execute_block(block, &Env::child(environment)),
            Stmt::While { condition, body } => {
                loop {
                    let value = self.evaluate(condition, environment)?;
                    if !self.truthy(&value)? {
                        break;
                    }
                    match self.execute_block(body, &Env::child(environment))? {
                        Flow::Next | Flow::Continue => {}
                        Flow::Break => break,
                        returned @ Flow::Return(_) => return Ok(returned),
                    }
                }
                Ok(Flow::Next)
            }
            Stmt::DoWhile { body, condition } => {
                loop {
                    match self.execute_block(body, &Env::child(environment))? {
                        Flow::Next | Flow::Continue => {}
                        Flow::Break => break,
                        returned @ Flow::Return(_) => return Ok(returned),
                    }
                    let value = self.evaluate(condition, environment)?;
                    if !self.truthy(&value)? {
                        break;
                    }
                }
                Ok(Flow::Next)
            }
            Stmt::Foreach {
                iterable,
                binding,
                body,
            } => {
                let iterable = self.evaluate(iterable, environment)?;
                let iterable = self.force(iterable)?;
                let items = match &**iterable.borrow() {
                    Value::List(items) => Some(items.borrow().clone()),
                    _ => None,
                };
                let items = if let Some(items) = items {
                    items
                } else {
                    let location = Location { line: 0, column: 0 };
                    let iterator = self.member(iterable, "__iterator__", location)?;
                    let iterator = self.invoke(iterator, Vec::new(), location, environment)?;
                    let mut items = Vec::new();
                    loop {
                        let has_next = self.member(iterator.clone(), "__has_next__", location)?;
                        let has_next = self.invoke(has_next, Vec::new(), location, environment)?;
                        if !self.truthy(&has_next)? {
                            break;
                        }
                        let next = self.member(iterator.clone(), "__next__", location)?;
                        items.push(self.invoke(next, Vec::new(), location, environment)?);
                    }
                    items
                };
                for item in items {
                    let iteration = Env::child(environment);
                    if let Some(name) = binding {
                        Env::define(&iteration, name.clone(), item);
                    }
                    match self.execute_block(body, &iteration)? {
                        Flow::Next | Flow::Continue => {}
                        Flow::Break => break,
                        returned @ Flow::Return(_) => return Ok(returned),
                    }
                }
                Ok(Flow::Next)
            }
            Stmt::Break => Ok(Flow::Break),
            Stmt::Continue => Ok(Flow::Continue),
            Stmt::Return(expressions) => {
                let mut values = Vec::new();
                for expression in expressions {
                    let value = self.evaluate(expression, environment)?;
                    match &**value.borrow() {
                        Value::Multi(multiple) => values.extend(multiple.iter().cloned()),
                        _ => values.push(value.clone()),
                    }
                }
                Ok(Flow::Return(values))
            }
            Stmt::Import {
                aliases,
                from,
                import_all,
            } => {
                if *import_all {
                    let module = self.load_module(from)?;
                    let module = self.force(module)?;
                    let Value::Namespace(namespace) = &**module.borrow() else {
                        return Err(RuntimeError::new("import target is not a module"));
                    };
                    let exports = namespace.borrow().values.clone();
                    for (name, value) in exports {
                        Env::define(environment, name, value);
                    }
                } else if from.is_empty() {
                    for alias in aliases {
                        let module = self.load_module(&alias.module)?;
                        Env::define(environment, alias.alias.clone(), module);
                    }
                } else {
                    let module = self.load_module(from)?;
                    for alias in aliases {
                        let value = self.module_member_path(module.clone(), &alias.module)?;
                        Env::define(environment, alias.alias.clone(), value);
                    }
                }
                Ok(Flow::Next)
            }
        }
    }

    fn declare(
        &mut self,
        declaration: &Declaration,
        environment: &Environment,
    ) -> Result<(), RuntimeError> {
        let mut values = Vec::new();
        for expression in &declaration.values {
            let value = self.evaluate(expression, environment)?;
            if declaration.values.len() == 1 && declaration.bindings.len() > 1 {
                match &**self.force(value.clone())?.borrow() {
                    Value::List(items) => values.extend(items.borrow().iter().cloned()),
                    Value::Multi(items) => values.extend(items.iter().cloned()),
                    _ => values.push(value),
                }
            } else {
                values.push(value);
            }
        }
        while values.len() < declaration.bindings.len() {
            values.push(cell(Value::None));
        }
        for (index, binding) in declaration.bindings.iter().enumerate() {
            let value = values
                .get(index)
                .cloned()
                .unwrap_or_else(|| cell(Value::None));
            self.bind(binding, value, environment)?;
        }
        Ok(())
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
                    copy_variable(value)
                };
                Env::define(environment, name.clone(), value);
            }
            Binding::Destructure(bindings) => {
                let forced = self.force(value)?;
                let items = match &**forced.borrow() {
                    Value::List(items) => items.borrow().clone(),
                    Value::Multi(items) => items.clone(),
                    _ => return Err(RuntimeError::new("cannot unpack non-list value")),
                };
                for (index, binding) in bindings.iter().enumerate() {
                    let item = items
                        .get(index)
                        .cloned()
                        .unwrap_or_else(|| cell(Value::None));
                    self.bind(binding, item, environment)?;
                }
            }
        }
        Ok(())
    }

    fn evaluate(
        &mut self,
        expression: &Expr,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        match expression {
            Expr::Literal(literal) => Ok(cell(match literal {
                Literal::None => Value::None,
                Literal::Bool(value) => Value::Bool(*value),
                Literal::Integer(value) => Value::Integer(*value),
                Literal::Float(value) => Value::Float(*value),
                Literal::String(value) => Value::String(value.clone()),
            })),
            Expr::Identifier(name, location) => Env::find(environment, name)
                .ok_or_else(|| RuntimeError::at(*location, format!("no variable named {name}"))),
            Expr::List(expressions) => {
                let mut values = Vec::new();
                for expression in expressions {
                    let value = self.evaluate(expression, environment)?;
                    values.push(copy_variable(value));
                }
                Ok(cell(list_value(values)))
            }
            Expr::Lambda { parameters, body } => Ok(cell(Value::Function(Rc::new(Function {
                parameters: parameters.clone(),
                body: body.clone(),
                closure: Rc::clone(environment),
            })))),
            Expr::Unary {
                operator,
                operand,
                location,
            } => {
                let operand = self.evaluate(operand, environment)?;
                self.unary(operator, operand, *location, environment)
            }
            Expr::Binary {
                left,
                operator,
                right,
                location,
            } if operator == ":" => {
                let target = self.lvalue(left, environment, true)?;
                let value = self.evaluate(right, environment)?;
                let value = self.force(value)?;
                *target.borrow_mut() = value.borrow().clone();
                Ok(target)
            }
            Expr::Binary {
                left,
                operator,
                right,
                location,
            } => {
                let left = self.evaluate(left, environment)?;
                if operator == "and" && !self.truthy(&left)? {
                    return Ok(cell(Value::Bool(false)));
                }
                if operator == "or" && self.truthy(&left)? {
                    return Ok(cell(Value::Bool(true)));
                }
                let right = self.evaluate(right, environment)?;
                self.binary(operator, left, right, *location, environment)
            }
            Expr::Call {
                callee,
                arguments,
                location,
            } => {
                let callee = self.evaluate(callee, environment)?;
                self.call(callee, arguments, *location, environment)
            }
            Expr::Member {
                object,
                name,
                location,
            } => {
                let receiver = self.evaluate(object, environment)?;
                self.member(receiver, name, *location)
            }
            Expr::Index {
                object,
                index,
                location,
            } => self.index(object, index, *location, environment),
            Expr::Conditional {
                condition,
                then_value,
                else_value,
                ..
            } => {
                let condition = self.evaluate(condition, environment)?;
                if self.truthy(&condition)? {
                    self.evaluate(then_value, environment)
                } else {
                    self.evaluate(else_value, environment)
                }
            }
            Expr::Delay(expression) => Ok(cell(Value::Thunk(Thunk {
                expression: (**expression).clone(),
                environment: Rc::clone(environment),
                cached: Rc::new(RefCell::new(None)),
            }))),
            Expr::Dict(entries) => {
                let mut values = HashMap::new();
                for (key, value) in entries {
                    let key = self.evaluate(key, environment)?;
                    let key = self.key(&key)?;
                    let value = self.evaluate(value, environment)?;
                    values.insert(key, copy_variable(value));
                }
                Ok(cell(dict_value(values)))
            }
            Expr::Enum(entries) => {
                let namespace = Env::child(environment);
                for (name, value) in entries {
                    Env::define(&namespace, name.clone(), cell(Value::Integer(*value)));
                }
                Ok(cell(Value::Namespace(namespace)))
            }
            Expr::Match {
                value,
                arms,
                fallback,
            } => {
                let value = self.evaluate(value, environment)?;
                let value = self.force(value)?;
                for (keys, result) in arms {
                    for key in keys {
                        let key = self.evaluate(key, environment)?;
                        let key = self.force(key)?;
                        if values_equal(&value, &key) {
                            return self.evaluate(result, environment);
                        }
                    }
                }
                if let Some(fallback) = fallback {
                    self.evaluate(fallback, environment)
                } else {
                    Ok(cell(Value::None))
                }
            }
            Expr::Class {
                name,
                bases,
                members,
            } => self.class_value(name.as_deref(), bases, members, environment),
        }
    }

    fn lvalue(
        &mut self,
        expression: &Expr,
        environment: &Environment,
        create: bool,
    ) -> Result<Cell, RuntimeError> {
        let value = match expression {
            Expr::Identifier(name, location) => {
                if let Some(value) = Env::find(environment, name) {
                    value
                } else if create {
                    let value = cell(Value::None);
                    Env::define(environment, name.clone(), value.clone());
                    value
                } else {
                    return Err(RuntimeError::at(
                        *location,
                        format!("no variable named {name}"),
                    ));
                }
            }
            Expr::Index {
                object,
                index,
                location,
            } => self.index(object, index, *location, environment)?,
            Expr::Member {
                object,
                name,
                location,
            } => {
                let object = self.evaluate(object, environment)?;
                self.member_lvalue(object, name, *location, create)?
            }
            _ => return Err(RuntimeError::new("expression is not assignable")),
        };
        self.reference(value)
    }

    fn index(
        &mut self,
        object: &Expr,
        index: &Expr,
        location: Location,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let object = self.evaluate(object, environment)?;
        let object = self.force(object)?;
        let index = self.evaluate(index, environment)?;
        let index = self.force(index)?;
        let index_value = index.borrow().value.clone();
        match (&mut **object.borrow_mut(), index_value) {
            (Value::List(items), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                items
                    .borrow()
                    .get(index)
                    .cloned()
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::String(value), Value::Integer(index)) => {
                let index = usize::try_from(index)
                    .map_err(|_| RuntimeError::at(location, "index should be non-negative"))?;
                value
                    .chars()
                    .nth(index)
                    .map(|character| cell(Value::String(character.to_string())))
                    .ok_or_else(|| RuntimeError::at(location, "index out of bounds"))
            }
            (Value::Dict(values), key) => {
                let key = value_key(&key)?;
                Ok(values
                    .borrow_mut()
                    .entry(key)
                    .or_insert_with(|| cell(Value::None))
                    .clone())
            }
            (_, Value::Integer(_)) => Err(RuntimeError::at(location, "object is not indexable")),
            _ => Err(RuntimeError::at(location, "index should be integer")),
        }
    }

    fn member(
        &mut self,
        receiver: Cell,
        name: &str,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        let receiver = self.force(receiver)?;
        let direct_member = match &**receiver.borrow() {
            Value::Namespace(namespace) => Env::find(namespace, name),
            Value::Function(function) => Env::find(&function.closure, name),
            Value::Class(class) => {
                if let Some(value) = class_member(class, name) {
                    let member = value.borrow().value.clone();
                    if let Value::Function(function) = member {
                        return Ok(cell(Value::UserMethod {
                            function,
                            receiver: receiver.clone(),
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
                    if let Value::Function(function) = member {
                        return Ok(cell(Value::UserMethod {
                            function,
                            receiver: receiver.clone(),
                        }));
                    }
                    Some(value)
                } else if let Some(value) = class_member(class, name) {
                    let member = value.borrow().value.clone();
                    if let Value::Function(function) = member {
                        return Ok(cell(Value::UserMethod {
                            function,
                            receiver: receiver.clone(),
                        }));
                    }
                    Some(value)
                } else {
                    None
                }
            }
            _ => None,
        };
        if let Some(member) = direct_member {
            return Ok(member);
        }
        let supported = match &**receiver.borrow() {
            Value::List(_) => matches!(
                name,
                "empty" | "size" | "push" | "pop" | "pop_front" | "front" | "back" | "clear"
            ),
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
                receiver,
                name: name.to_owned(),
            }))
        } else if let Value::Dict(values) = &mut **receiver.borrow_mut() {
            Ok(values
                .borrow_mut()
                .entry(format!("s{name}"))
                .or_insert_with(|| cell(Value::None))
                .clone())
        } else {
            Err(RuntimeError::at(
                location,
                format!("no member named {name}"),
            ))
        }
    }

    fn call(
        &mut self,
        callee: Cell,
        arguments: &[Argument],
        location: Location,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let callee = self.force(callee)?;
        let arguments = self.arguments(arguments, environment)?;
        self.invoke(callee, arguments, location, environment)
    }

    fn invoke(
        &mut self,
        callee: Cell,
        arguments: Vec<Cell>,
        location: Location,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let callee = self.force(callee)?;
        let value = callee.borrow().value.clone();
        match value {
            Value::Function(function) => self.call_function(&function, arguments, location),
            Value::Builtin(builtin) => self.call_builtin(builtin, arguments, environment),
            Value::BoundMethod { receiver, name } => self.call_method(receiver, &name, arguments),
            Value::UserMethod { function, receiver } => {
                let mut arguments = arguments;
                arguments.insert(0, receiver);
                self.call_function(&function, arguments, location)
            }
            Value::Class(class) => self.instantiate(class, arguments, location),
            _ => Err(RuntimeError::at(location, "object is not callable")),
        }
    }

    fn arguments(
        &mut self,
        arguments: &[Argument],
        environment: &Environment,
    ) -> Result<Vec<Cell>, RuntimeError> {
        let mut values = Vec::new();
        for argument in arguments {
            let value = self.evaluate(&argument.value, environment)?;
            if argument.unpack {
                let value = self.force(value)?;
                match &**value.borrow() {
                    Value::List(items) => values.extend(items.borrow().iter().cloned()),
                    Value::Multi(items) => values.extend(items.iter().cloned()),
                    _ => return Err(RuntimeError::new("cannot unpack non-list argument")),
                }
            } else {
                values.push(value);
            }
        }
        Ok(values)
    }

    fn call_function(
        &mut self,
        function: &Function,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        let call_environment = Env::child(&function.closure);
        let mut argument_index = 0;
        for parameter in &function.parameters {
            if parameter.variadic {
                let mut items = Vec::new();
                for argument in &arguments[argument_index..] {
                    if parameter.by_reference {
                        items.push(self.reference(argument.clone())?);
                    } else {
                        items.push(copy_variable(argument.clone()));
                    }
                }
                Env::define(
                    &call_environment,
                    parameter.name.clone(),
                    cell(list_value(items)),
                );
                argument_index = arguments.len();
                continue;
            }
            let argument = if let Some(argument) = arguments.get(argument_index) {
                argument_index += 1;
                argument.clone()
            } else if let Some(default) = &parameter.default {
                self.evaluate(default, &function.closure)?
            } else {
                return Err(RuntimeError::at(location, "not enough arguments"));
            };
            let value = if parameter.by_reference {
                self.reference(argument)?
            } else {
                copy_variable(argument)
            };
            Env::define(&call_environment, parameter.name.clone(), value);
        }
        if argument_index < arguments.len() {
            return Err(RuntimeError::at(location, "too many arguments"));
        }
        match self.execute_block(&function.body, &call_environment)? {
            Flow::Next => Ok(cell(Value::None)),
            Flow::Return(values) if values.is_empty() => Ok(cell(Value::None)),
            Flow::Return(values) if values.len() == 1 => Ok(values[0].clone()),
            Flow::Return(values) => Ok(cell(Value::Multi(values))),
            Flow::Break => Err(RuntimeError::new("break outside loop")),
            Flow::Continue => Err(RuntimeError::new("continue outside loop")),
        }
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
                let value = self.force(first())?;
                if !matches!(**value.borrow(), Value::None) {
                    let rendered = self.stringify(&value)?;
                    self.output.push_str(&rendered);
                }
                if matches!(builtin, Builtin::Println) {
                    self.output.push('\n');
                }
                Ok(cell(Value::None))
            }
            Builtin::Str => {
                let value = self.force(first())?;
                Ok(cell(Value::String(self.stringify(&value)?)))
            }
            Builtin::Type => {
                let value = self.force(first())?;
                Ok(cell(Value::String(type_name(&value.borrow()).to_owned())))
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
                let source = self.force(first())?;
                let source = match &**source.borrow() {
                    Value::String(source) => source.clone(),
                    _ => return Err(RuntimeError::new("eval expects a string")),
                };
                let program = Parser::new(&format!("return {source};"), "<eval>")?.parse()?;
                match self.execute_block(&program, environment)? {
                    Flow::Return(values) if values.len() == 1 => Ok(values[0].clone()),
                    Flow::Return(values) => Ok(cell(Value::Multi(values))),
                    _ => Ok(cell(Value::None)),
                }
            }
            Builtin::Id => {
                let value = self.force(first())?;
                Ok(cell(Value::Integer(identity_id(&value))))
            }
            Builtin::Args => Ok(cell(list_value(
                self.arguments
                    .iter()
                    .map(|argument| cell(Value::String(argument.clone())))
                    .collect(),
            ))),
            Builtin::Open => {
                if arguments.len() != 2 {
                    return Err(RuntimeError::new("function open need 2 arguments"));
                }
                let path = self.force(arguments[0].clone())?;
                let path = self.path_from_value(&path)?;
                let mode = self.force(arguments[1].clone())?;
                let mode = match **mode.borrow() {
                    Value::Integer(mode) => mode,
                    _ => return Err(RuntimeError::new("file mode should be integer")),
                };
                let mut options = OpenOptions::new();
                let append = mode & (1 << 0) != 0;
                let read = mode & (1 << 2) != 0;
                let write = mode & (1 << 3) != 0;
                let truncate = mode & (1 << 4) != 0;
                options
                    .append(append)
                    .read(read)
                    .write(write)
                    .truncate(truncate)
                    .create(write || append);
                let mut file = options
                    .open(path)
                    .map_err(|error| RuntimeError::new(format!("cannot open file: {error}")))?;
                if mode & (1 << 5) != 0 {
                    file.seek(SeekFrom::End(0))
                        .map_err(|error| RuntimeError::new(error.to_string()))?;
                }
                Ok(cell(Value::File(Rc::new(RefCell::new(RuntimeFile {
                    file: Some(file),
                    eof: false,
                })))))
            }
            Builtin::CurrentPath => Ok(cell(Value::Path(
                std::env::current_dir().unwrap_or_else(|_| PathBuf::from(".")),
            ))),
            Builtin::IsDirectory => {
                let path = self.path_from_value(&first())?;
                Ok(cell(Value::Bool(path.is_dir())))
            }
            Builtin::ReadDirectory => {
                let path = self.path_from_value(&first())?;
                let mut entries = Vec::new();
                for entry in
                    fs::read_dir(path).map_err(|error| RuntimeError::new(error.to_string()))?
                {
                    let entry = entry.map_err(|error| RuntimeError::new(error.to_string()))?;
                    entries.push(cell(Value::Path(entry.path())));
                }
                Ok(cell(list_value(entries)))
            }
            Builtin::Input => {
                let mut line = String::new();
                std::io::stdin().read_line(&mut line)?;
                while matches!(line.chars().last(), Some('\n' | '\r')) {
                    line.pop();
                }
                Ok(cell(Value::String(line)))
            }
            Builtin::Exit => {
                self.halted = true;
                self.pending_steps.clear();
                Ok(cell(Value::None))
            }
            Builtin::CallWithCurrentContinuation => Err(RuntimeError::new(
                "call_with_current_continuation requires CPS execution",
            )),
        }
    }

    fn call_method(
        &mut self,
        receiver: Cell,
        name: &str,
        arguments: Vec<Cell>,
    ) -> Result<Cell, RuntimeError> {
        let mut borrowed = receiver.borrow_mut();
        match (&mut **borrowed, name) {
            (Value::List(items), "empty") => Ok(cell(Value::Bool(items.borrow().is_empty()))),
            (Value::List(items), "size") => Ok(cell(Value::Integer(items.borrow().len() as i64))),
            (Value::List(items), "push") => {
                let value = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                items.borrow_mut().push(copy_variable(self.force(value)?));
                Ok(cell(Value::None))
            }
            (Value::List(items), "pop") => items
                .borrow_mut()
                .pop()
                .ok_or_else(|| RuntimeError::new("pop from empty list")),
            (Value::List(items), "pop_front") => {
                let mut items = items.borrow_mut();
                if items.is_empty() {
                    Err(RuntimeError::new("pop from empty list"))
                } else {
                    Ok(items.remove(0))
                }
            }
            (Value::List(items), "front") => items
                .borrow()
                .first()
                .cloned()
                .ok_or_else(|| RuntimeError::new("front of empty list")),
            (Value::List(items), "back") => items
                .borrow()
                .last()
                .cloned()
                .ok_or_else(|| RuntimeError::new("back of empty list")),
            (Value::List(items), "clear") => {
                items.borrow_mut().clear();
                Ok(cell(Value::None))
            }
            (Value::String(value), "size") => Ok(cell(Value::Integer(value.len() as i64))),
            (Value::String(value), "to_int") => value
                .parse()
                .map(|value| cell(Value::Integer(value)))
                .map_err(|_| RuntimeError::new("invalid integer")),
            (Value::Integer(value), "to_str") => Ok(cell(Value::String(value.to_string()))),
            (Value::Dict(values), "empty") => Ok(cell(Value::Bool(values.borrow().is_empty()))),
            (Value::Dict(values), "size") => Ok(cell(Value::Integer(values.borrow().len() as i64))),
            (Value::Dict(values), "at") => {
                let key = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                let key = self.key(&key)?;
                Ok(values
                    .borrow_mut()
                    .entry(key)
                    .or_insert_with(|| cell(Value::None))
                    .clone())
            }
            (Value::Dict(values), "insert") => {
                if arguments.len() != 2 {
                    return Err(RuntimeError::new("dict.insert expects 2 arguments"));
                }
                let key = self.key(&arguments[0])?;
                let value = self.force(arguments[1].clone())?;
                values.borrow_mut().insert(key, copy_variable(value));
                Ok(cell(Value::None))
            }
            (Value::Dict(values), "erase") => {
                let key = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                let key = self.key(&key)?;
                values.borrow_mut().remove(&key);
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
            (Value::Path(path), "is_directory") => Ok(cell(Value::Bool(path.is_dir()))),
            _ => Err(RuntimeError::new(format!("no member named {name}"))),
        }
    }

    fn unary(
        &mut self,
        operator: &str,
        operand: Cell,
        location: Location,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let operand = self.force(operand)?;
        let value = operand.borrow().value.clone();
        match (operator, value) {
            ("not" | "!", _) => Ok(cell(Value::Bool(!self.truthy(&operand)?))),
            ("-", Value::Integer(value)) => Ok(cell(Value::Integer(-value))),
            ("-", Value::Float(value)) => Ok(cell(Value::Float(-value))),
            ("~", Value::Integer(value)) => Ok(cell(Value::Integer(!value))),
            _ => {
                let function = Env::find(environment, operator).ok_or_else(|| {
                    RuntimeError::at(location, format!("no prefix operator {operator}"))
                })?;
                self.call_function_value(function, vec![operand], location)
            }
        }
    }

    fn binary(
        &mut self,
        operator: &str,
        left: Cell,
        right: Cell,
        location: Location,
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let left = self.force(left)?;
        let right = self.force(right)?;
        let left_value = left.borrow().value.clone();
        let right_value = right.borrow().value.clone();
        let result = match (operator, left_value, right_value) {
            ("+", Value::Integer(left), Value::Integer(right)) => Value::Integer(left + right),
            ("-", Value::Integer(left), Value::Integer(right)) => Value::Integer(left - right),
            ("*", Value::Integer(left), Value::Integer(right)) => Value::Integer(left * right),
            ("/", Value::Integer(left), Value::Integer(right)) => Value::Integer(left / right),
            ("%", Value::Integer(left), Value::Integer(right)) => Value::Integer(left % right),
            ("+", Value::Float(left), Value::Float(right)) => Value::Float(left + right),
            ("-", Value::Float(left), Value::Float(right)) => Value::Float(left - right),
            ("*", Value::Float(left), Value::Float(right)) => Value::Float(left * right),
            ("/", Value::Float(left), Value::Float(right)) => Value::Float(left / right),
            ("+", Value::String(left), Value::String(right)) => {
                Value::String(format!("{left}{right}"))
            }
            ("+", Value::List(left), Value::List(right)) => {
                let mut combined = left.borrow().clone();
                combined.extend(right.borrow().iter().cloned());
                list_value(combined)
            }
            ("&", Value::Integer(left), Value::Integer(right)) => Value::Integer(left & right),
            ("|", Value::Integer(left), Value::Integer(right)) => Value::Integer(left | right),
            ("^", Value::Integer(left), Value::Integer(right)) => Value::Integer(left ^ right),
            ("<<", Value::Integer(left), Value::Integer(right)) => Value::Integer(left << right),
            (">>", Value::Integer(left), Value::Integer(right)) => Value::Integer(left >> right),
            ("and", _, _) => Value::Bool(self.truthy(&left)? && self.truthy(&right)?),
            ("or", _, _) => Value::Bool(self.truthy(&left)? || self.truthy(&right)?),
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
            (">", Value::Integer(left), Value::Integer(right)) => Value::Bool(left > right),
            (">=", Value::Integer(left), Value::Integer(right)) => Value::Bool(left >= right),
            ("<", Value::Float(left), Value::Float(right)) => Value::Bool(left < right),
            ("<=", Value::Float(left), Value::Float(right)) => Value::Bool(left <= right),
            (">", Value::Float(left), Value::Float(right)) => Value::Bool(left > right),
            (">=", Value::Float(left), Value::Float(right)) => Value::Bool(left >= right),
            ("<", Value::String(left), Value::String(right)) => Value::Bool(left < right),
            ("<=", Value::String(left), Value::String(right)) => Value::Bool(left <= right),
            _ => {
                let function = Env::find(environment, operator).ok_or_else(|| {
                    RuntimeError::at(location, format!("no infix operator {operator}"))
                })?;
                return self.call_function_value(function, vec![left, right], location);
            }
        };
        Ok(cell(result))
    }

    fn call_function_value(
        &mut self,
        function: Cell,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        let function = self.force(function)?;
        let value = function.borrow().value.clone();
        match value {
            Value::Function(function) => self.call_function(&function, arguments, location),
            _ => Err(RuntimeError::at(location, "operator is not callable")),
        }
    }

    fn force(&mut self, value: Cell) -> Result<Cell, RuntimeError> {
        let thunk = match &**value.borrow() {
            Value::Thunk(thunk) => Some(thunk.clone()),
            _ => None,
        };
        let Some(thunk) = thunk else {
            return Ok(value);
        };
        if let Some(cached) = thunk.cached.borrow().clone() {
            return self.force(cached);
        }
        let result = self.evaluate(&thunk.expression, &thunk.environment)?;
        *thunk.cached.borrow_mut() = Some(result.clone());
        self.force(result)
    }

    fn reference(&mut self, value: Cell) -> Result<Cell, RuntimeError> {
        self.force(value)
    }

    fn truthy(&mut self, value: &Cell) -> Result<bool, RuntimeError> {
        let value = self.force(value.clone())?;
        let result = match &**value.borrow() {
            Value::Bool(value) => *value,
            Value::Integer(value) => *value != 0,
            Value::Float(value) => *value != 0.0,
            Value::String(value) => !value.is_empty(),
            Value::List(value) => !value.borrow().is_empty(),
            Value::Multi(value) => !value.is_empty(),
            Value::Dict(value) => !value.borrow().is_empty(),
            Value::None
            | Value::File(_)
            | Value::Path(_)
            | Value::Continuation(_)
            | Value::Function(_)
            | Value::Namespace(_)
            | Value::Class(_)
            | Value::Instance { .. }
            | Value::UserMethod { .. }
            | Value::Builtin(_)
            | Value::BoundMethod { .. }
            | Value::Thunk(_) => return Err(RuntimeError::new("cannot translate to bool")),
        };
        Ok(result)
    }

    fn stringify(&mut self, value: &Cell) -> Result<String, RuntimeError> {
        let value = self.force(value.clone())?;
        let borrowed = value.borrow();
        Ok(match &**borrowed {
            Value::None => "<no definition of to_str>".to_owned(),
            Value::Bool(value) => value.to_string(),
            Value::Integer(value) => value.to_string(),
            Value::Float(value) => format!("{value:.6}"),
            Value::String(value) => value.clone(),
            Value::List(items) => {
                let items = items.borrow().clone();
                drop(borrowed);
                let mut rendered = Vec::new();
                for item in items {
                    rendered.push(self.stringify(&item)?);
                }
                return Ok(format!("[{}]", rendered.join(", ")));
            }
            Value::Multi(items) => {
                let items = items.clone();
                drop(borrowed);
                let mut rendered = Vec::new();
                for item in items {
                    rendered.push(self.stringify(&item)?);
                }
                return Ok(format!("[{}]", rendered.join(", ")));
            }
            Value::Dict(values) => {
                let values = values.borrow().clone();
                drop(borrowed);
                let mut entries = Vec::new();
                for (key, value) in values {
                    entries.push(format!(
                        "{} => {}",
                        display_key(&key),
                        self.stringify(&value)?
                    ));
                }
                entries.sort();
                return Ok(format!("{{ {} }}", entries.join(", ")));
            }
            Value::File(_) => "<no definition of to_str>".to_owned(),
            Value::Path(path) => path.display().to_string(),
            Value::Continuation(_) => "<no definition of to_str>".to_owned(),
            Value::Function(_) => "<function>".to_owned(),
            Value::Namespace(_) | Value::Class(_) | Value::Instance { .. } => {
                "<no definition of to_str>".to_owned()
            }
            Value::UserMethod { .. } | Value::BoundMethod { .. } => {
                "<no definition of to_str>".to_owned()
            }
            Value::Builtin(_) => "<builtin-function>".to_owned(),
            Value::Thunk(_) => unreachable!("thunks are forced before rendering"),
        })
    }

    fn member_lvalue(
        &mut self,
        object: Cell,
        name: &str,
        location: Location,
        create: bool,
    ) -> Result<Cell, RuntimeError> {
        let object = self.force(object)?;
        if let Value::Dict(values) = &mut **object.borrow_mut() {
            let key = format!("s{name}");
            if let Some(value) = values.borrow().get(&key) {
                return self.reference(value.clone());
            }
            if create {
                let value = cell(Value::None);
                values.borrow_mut().insert(key, value.clone());
                return Ok(value);
            }
            return Err(RuntimeError::at(
                location,
                format!("no member named {name}"),
            ));
        }
        let target_environment = match &**object.borrow() {
            Value::Namespace(environment) => Some(environment.clone()),
            Value::Function(function) => Some(function.closure.clone()),
            Value::Class(class) => Some(class.members.clone()),
            Value::Instance { fields, .. } => Some(fields.clone()),
            _ => None,
        };
        let Some(target_environment) = target_environment else {
            return Err(RuntimeError::at(location, "member is not assignable"));
        };
        if let Some(value) = Env::find(&target_environment, name) {
            return self.reference(value);
        }
        if create {
            let value = cell(Value::None);
            Env::define(&target_environment, name.to_owned(), value.clone());
            Ok(value)
        } else {
            Err(RuntimeError::at(
                location,
                format!("no member named {name}"),
            ))
        }
    }

    fn class_value(
        &mut self,
        _name: Option<&str>,
        bases: &[Argument],
        members: &[Declaration],
        environment: &Environment,
    ) -> Result<Cell, RuntimeError> {
        let mut base_classes = Vec::new();
        for base in bases {
            let value = self.evaluate(&base.value, environment)?;
            let value = self.force(value)?;
            match &**value.borrow() {
                Value::Class(class) => base_classes.push(class.clone()),
                _ => return Err(RuntimeError::new("class base must be a class")),
            }
        }
        let class_environment = Env::child(environment);
        let mut base_constructors = Vec::new();
        for base in &base_classes {
            if let Some(constructor) = class_member(base, "__init__") {
                base_constructors.push(constructor);
            }
        }
        Env::define(
            &class_environment,
            "bctors".to_owned(),
            cell(list_value(base_constructors)),
        );
        for member in members {
            self.declare(member, &class_environment)?;
        }
        Ok(cell(Value::Class(Rc::new(Class {
            members: class_environment,
            bases: base_classes,
        }))))
    }

    fn instantiate(
        &mut self,
        class: Rc<Class>,
        arguments: Vec<Cell>,
        location: Location,
    ) -> Result<Cell, RuntimeError> {
        let instance = cell(Value::Instance {
            class: class.clone(),
            fields: clone_class_members(&class),
        });
        if let Some(constructor) = class_member(&class, "__init__") {
            let function = constructor.borrow().value.clone();
            let Value::Function(function) = function else {
                return Err(RuntimeError::at(location, "__init__ must be callable"));
            };
            let mut constructor_arguments = arguments;
            constructor_arguments.insert(0, instance.clone());
            self.call_function(&function, constructor_arguments, location)?;
        } else if !arguments.is_empty() {
            return Err(RuntimeError::at(location, "too many arguments"));
        }
        Ok(instance)
    }

    fn key(&mut self, value: &Cell) -> Result<String, RuntimeError> {
        let value = self.force(value.clone())?;
        value_key(&value.borrow())
    }

    fn call_file_method(
        &mut self,
        runtime_file: &Rc<RefCell<RuntimeFile>>,
        method: &str,
        arguments: Vec<Cell>,
    ) -> Result<Cell, RuntimeError> {
        let mut runtime_file = runtime_file.borrow_mut();
        match method {
            "good" => Ok(cell(Value::Bool(runtime_file.file.is_some()))),
            "eof" => Ok(cell(Value::Bool(runtime_file.eof))),
            "close" => {
                runtime_file.file = None;
                Ok(cell(Value::None))
            }
            "flush" => {
                open_file(&mut runtime_file)?.flush()?;
                Ok(cell(Value::None))
            }
            "read" => {
                let mut byte = [0_u8; 1];
                let count = open_file(&mut runtime_file)?.read(&mut byte)?;
                runtime_file.eof = count == 0;
                let value = if count == 0 {
                    String::new()
                } else {
                    char::from(byte[0]).to_string()
                };
                Ok(cell(Value::String(value)))
            }
            "readline" => {
                let mut line = String::new();
                let count = {
                    let file = open_file(&mut runtime_file)?;
                    BufReader::new(file).read_line(&mut line)?
                };
                runtime_file.eof = count == 0;
                if line.ends_with('\n') {
                    line.pop();
                    if line.ends_with('\r') {
                        line.pop();
                    }
                }
                Ok(cell(Value::String(line)))
            }
            "write" => {
                let value = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::None));
                let value = self.stringify(&value)?;
                open_file(&mut runtime_file)?.write_all(value.as_bytes())?;
                Ok(cell(Value::None))
            }
            "tellg" | "tellp" => {
                let position = open_file(&mut runtime_file)?.stream_position()?;
                Ok(cell(Value::Integer(
                    i64::try_from(position).unwrap_or(i64::MAX),
                )))
            }
            "seekg" | "seekp" => {
                let position = arguments
                    .first()
                    .cloned()
                    .unwrap_or_else(|| cell(Value::Integer(0)));
                let position = self.force(position)?;
                let position = match **position.borrow() {
                    Value::Integer(position) => u64::try_from(position)
                        .map_err(|_| RuntimeError::new("negative seek position"))?,
                    _ => return Err(RuntimeError::new("seek position should be integer")),
                };
                open_file(&mut runtime_file)?.seek(SeekFrom::Start(position))?;
                runtime_file.eof = false;
                Ok(cell(Value::None))
            }
            _ => Err(RuntimeError::new(format!("no member named {method}"))),
        }
    }

    fn path_from_value(&mut self, value: &Cell) -> Result<PathBuf, RuntimeError> {
        let value = self.force(value.clone())?;
        let mut path = match &**value.borrow() {
            Value::Path(path) => path.clone(),
            Value::String(path) => PathBuf::from(path),
            _ => PathBuf::from(self.stringify(&value)?),
        };
        if path.is_relative() {
            path = std::env::current_dir()
                .unwrap_or_else(|_| PathBuf::from("."))
                .join(path);
        }
        Ok(path)
    }

    fn load_module(&mut self, parts: &[ModulePart]) -> Result<Cell, RuntimeError> {
        let Some(first) = parts.first() else {
            return Err(RuntimeError::new("empty module path"));
        };
        let module = match first {
            ModulePart::Name(name) => {
                if let Ok(path) = self.resolve_named_module(name) {
                    self.load_module_path(&path)?
                } else {
                    self.load_standard_module(name)?
                }
            }
            ModulePart::Path(path) => {
                let path = self.current_directory().join(path);
                self.load_module_path(&path)?
            }
        };
        self.module_member_path(module, &parts[1..])
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

    fn resolve_named_module(&self, name: &str) -> Result<PathBuf, RuntimeError> {
        let current = self.current_directory();
        let working = std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."));
        let roots = [
            current,
            working.join("lib"),
            PathBuf::from("/usr/local/lib/anole"),
        ];
        for root in roots {
            let file = root.join(format!("{name}.anole"));
            if file.is_file() {
                return Ok(file);
            }
            let directory = root.join(name).join("__init__.anole");
            if directory.is_file() {
                return Ok(directory);
            }
        }
        Err(RuntimeError::new(format!("no module named {name}")))
    }

    fn load_module_path(&mut self, path: &Path) -> Result<Cell, RuntimeError> {
        let path = if path.extension().is_none() && path.is_dir() {
            path.join("__init__.anole")
        } else {
            path.to_path_buf()
        };
        let normalized = fs::canonicalize(&path).unwrap_or_else(|_| path.clone());
        if let Some(module) = self.modules.get(&normalized) {
            return Ok(module.clone());
        }
        if path.extension().is_some_and(|extension| extension == "so") {
            return self.load_native_module(&path, normalized);
        }
        let source = fs::read_to_string(&path)
            .map_err(|_| RuntimeError::new(format!("cannot open file {}", path.display())))?;
        let module_environment = Env::child(&self.globals);
        let module = cell(Value::Namespace(module_environment.clone()));
        self.modules.insert(normalized, module.clone());
        let program = Parser::new(&source, path.display().to_string())?.parse()?;
        self.directories.push(
            path.parent()
                .unwrap_or_else(|| Path::new("."))
                .to_path_buf(),
        );
        let execution = self.execute_block(&program, &module_environment);
        self.directories.pop();
        execution?;
        Ok(module)
    }

    fn load_native_module(
        &mut self,
        path: &Path,
        cache_key: PathBuf,
    ) -> Result<Cell, RuntimeError> {
        let module_environment = Env::child(&self.globals);
        match path.file_name().and_then(|name| name.to_str()) {
            Some("libenv.so") => Env::define(
                &module_environment,
                "__args".to_owned(),
                cell(Value::Builtin(Builtin::Args)),
            ),
            Some("libfileobject.so") => Env::define(
                &module_environment,
                "__open".to_owned(),
                cell(Value::Builtin(Builtin::Open)),
            ),
            Some("libpath.so") => {
                Env::define(
                    &module_environment,
                    "__current_path".to_owned(),
                    cell(Value::Builtin(Builtin::CurrentPath)),
                );
                Env::define(
                    &module_environment,
                    "__is_directory".to_owned(),
                    cell(Value::Builtin(Builtin::IsDirectory)),
                );
            }
            Some("libread_dir.so") => Env::define(
                &module_environment,
                "__read_dir".to_owned(),
                cell(Value::Builtin(Builtin::ReadDirectory)),
            ),
            Some(name) => {
                return Err(RuntimeError::new(format!(
                    "native module {name} is not available in the Rust runtime"
                )));
            }
            None => return Err(RuntimeError::new("invalid native module path")),
        }
        let module = cell(Value::Namespace(module_environment));
        self.modules.insert(cache_key, module.clone());
        Ok(module)
    }

    fn load_standard_module(&mut self, name: &str) -> Result<Cell, RuntimeError> {
        let cache_key = PathBuf::from(format!("<embedded>/{name}"));
        if let Some(module) = self.modules.get(&cache_key) {
            return Ok(module.clone());
        }
        let environment = Env::child(&self.globals);
        let module = cell(Value::Namespace(environment.clone()));
        self.modules.insert(cache_key, module.clone());
        match name {
            "env" => Env::define(
                &environment,
                "args".to_owned(),
                cell(Value::Builtin(Builtin::Args)),
            ),
            "file" => {
                let mode = Env::child(&self.globals);
                for (name, value) in [
                    ("app", 1 << 0),
                    ("binary", 1 << 1),
                    ("in", 1 << 2),
                    ("out", 1 << 3),
                    ("trunc", 1 << 4),
                    ("ate", 1 << 5),
                ] {
                    Env::define(&mode, name.to_owned(), cell(Value::Integer(value)));
                }
                Env::define(
                    &environment,
                    "mode".to_owned(),
                    cell(Value::Namespace(mode)),
                );
                Env::define(
                    &environment,
                    "open".to_owned(),
                    cell(Value::Builtin(Builtin::Open)),
                );
            }
            "os" => {
                let path = Env::child(&self.globals);
                Env::define(
                    &path,
                    "current_path".to_owned(),
                    cell(Value::Builtin(Builtin::CurrentPath)),
                );
                Env::define(
                    &path,
                    "is_directory".to_owned(),
                    cell(Value::Builtin(Builtin::IsDirectory)),
                );
                let read_directory = Env::child(&self.globals);
                Env::define(
                    &read_directory,
                    "read_dir".to_owned(),
                    cell(Value::Builtin(Builtin::ReadDirectory)),
                );
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
            "coroutine" => {
                const SOURCE: &str = include_str!("stdlib/coroutine.anole");
                let program =
                    Parser::new(SOURCE, "<embedded>/coroutine/__init__.anole")?.parse()?;
                self.execute_block(&program, &environment)?;
            }
            "debug" => {}
            _ => return Err(RuntimeError::new(format!("no module named {name}"))),
        }
        Ok(module)
    }

    fn current_directory(&self) -> PathBuf {
        self.directories
            .last()
            .cloned()
            .unwrap_or_else(|| PathBuf::from("."))
    }

    fn schedule(&mut self, step: Step) {
        self.pending_steps.push_back(step);
    }

    fn advance(&mut self, step: Step) -> Result<(), RuntimeError> {
        self.schedule(step);
        Ok(())
    }

    fn resume(&mut self, continuation: Continuation, value: Cell) -> Result<(), RuntimeError> {
        self.schedule(Rc::new(move |interpreter| {
            continuation(interpreter, value.clone())
        }));
        Ok(())
    }

    fn resume_return(
        &mut self,
        continuation: ReturnContinuation,
        values: Vec<Cell>,
    ) -> Result<(), RuntimeError> {
        self.schedule(Rc::new(move |interpreter| {
            continuation(interpreter, values.clone())
        }));
        Ok(())
    }
}

fn cell(value: Value) -> Cell {
    Rc::new(RefCell::new(ValueSlot::new(value)))
}

fn list_value(values: Vec<Cell>) -> Value {
    Value::List(Rc::new(RefCell::new(values)))
}

fn dict_value(values: HashMap<String, Cell>) -> Value {
    Value::Dict(Rc::new(RefCell::new(values)))
}

fn copy_variable(value: Cell) -> Cell {
    Rc::new(RefCell::new(value.borrow().clone()))
}

fn flatten_values(values: Vec<Cell>) -> Vec<Cell> {
    let mut flattened = Vec::new();
    for value in values {
        match &**value.borrow() {
            Value::Multi(items) => flattened.extend(items.iter().cloned()),
            _ => flattened.push(value.clone()),
        }
    }
    flattened
}

fn is_builtin_binary_operator(operator: &str) -> bool {
    matches!(
        operator,
        "or" | "and"
            | "|"
            | "^"
            | "&"
            | "="
            | "!="
            | "<"
            | "<="
            | ">"
            | ">="
            | "<<"
            | ">>"
            | "+"
            | "-"
            | "is"
            | "*"
            | "/"
            | "%"
    )
}

fn type_name(value: &Value) -> &'static str {
    match value {
        Value::None => "none",
        Value::Bool(_) => "boolean",
        Value::Integer(_) => "integer",
        Value::Float(_) => "float",
        Value::String(_) => "string",
        Value::List(_) => "list",
        Value::Dict(_) => "dict",
        Value::File(_) => "file",
        Value::Path(_) => "path",
        Value::Continuation(_) => "cont",
        Value::Function(_) => "func",
        Value::Namespace(_) => "anolemodule",
        Value::Class(_) => "class",
        Value::Instance { .. } => "instance",
        Value::UserMethod { .. } => "method",
        Value::Builtin(_) => "builtinfunc",
        Value::BoundMethod { .. } => "method",
        Value::Thunk(_) => "thunk",
        Value::Multi(_) => "list",
    }
}

fn values_equal(left: &Cell, right: &Cell) -> bool {
    match (&**left.borrow(), &**right.borrow()) {
        (Value::None, Value::None) => true,
        (Value::Bool(left), Value::Bool(right)) => left == right,
        (Value::Integer(left), Value::Integer(right)) => left == right,
        (Value::Float(left), Value::Float(right)) => left == right,
        (Value::String(left), Value::String(right)) => left == right,
        (Value::Function(left), Value::Function(right)) => Rc::ptr_eq(left, right),
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
        Value::None => 1,
        Value::Bool(false) => 2,
        Value::Bool(true) => 3,
        _ => Rc::as_ptr(&value.borrow().identity) as usize as i64,
    }
}

fn class_member(class: &Class, name: &str) -> Option<Cell> {
    Env::find(&class.members, name)
        .or_else(|| class.bases.iter().find_map(|base| class_member(base, name)))
}

fn value_key(value: &Value) -> Result<String, RuntimeError> {
    match value {
        Value::None => Ok("n".to_owned()),
        Value::Bool(value) => Ok(format!("b{value}")),
        Value::Integer(value) => Ok(format!("i{value}")),
        Value::Float(value) => Ok(format!("f{value}")),
        Value::String(value) => Ok(format!("s{value}")),
        Value::Path(value) => Ok(format!("p{}", value.display())),
        Value::List(values) => {
            let values = values.borrow();
            let mut key = String::from("l[");
            for value in values.iter() {
                key.push_str(&value_key(&value.borrow())?);
                key.push(',');
            }
            key.push(']');
            Ok(key)
        }
        Value::Multi(values) => {
            let mut key = String::from("l[");
            for value in values {
                key.push_str(&value_key(&value.borrow())?);
                key.push(',');
            }
            key.push(']');
            Ok(key)
        }
        Value::Dict(values) => {
            let mut keys: Vec<_> = values.borrow().keys().cloned().collect();
            keys.sort();
            Ok(format!("d{{{}}}", keys.join(",")))
        }
        _ => Err(RuntimeError::new("value cannot be used as a dict key")),
    }
}

fn open_file(runtime_file: &mut RuntimeFile) -> Result<&mut File, RuntimeError> {
    runtime_file
        .file
        .as_mut()
        .ok_or_else(|| RuntimeError::new("file is closed"))
}

impl From<std::io::Error> for RuntimeError {
    fn from(error: std::io::Error) -> Self {
        Self::new(error.to_string())
    }
}

fn display_key(key: &str) -> &str {
    key.get(1..).unwrap_or(key)
}

fn clone_class_members(class: &Class) -> Environment {
    let fields = Env::root();
    for base in &class.bases {
        let base_fields = clone_class_members(base);
        for (name, value) in base_fields.borrow().values.clone() {
            Env::define(&fields, name, value);
        }
    }
    for (name, value) in class.members.borrow().values.clone() {
        Env::define(&fields, name, copy_variable(value));
    }
    fields
}
