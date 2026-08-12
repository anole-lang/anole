//! Anole bytecode generation and `.ir` serialization.
//!
//! The format is native-endian and uses fixed-width 64-bit integer fields.

use std::collections::BTreeMap;
use std::fs;
use std::io;
use std::path::Path;

use crate::Location;
use crate::ast::{Binding, Block, Declaration, Expr, Literal, ModulePart, Stmt};
use crate::lexer::{symbol_from_bytes, symbol_to_bytes};

const MAGIC: u64 = 20_210_213;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
#[repr(u8)]
pub(crate) enum Opcode {
    PlaceHolder,
    Pop,
    Import,
    ImportPath,
    ImportAll,
    ImportPart,
    Load,
    LoadConst,
    LoadMember,
    Store,
    StoreRef,
    StoreLocal,
    NewScope,
    EndScope,
    CallAc,
    Call,
    FastCall,
    Return,
    ReturnNone,
    Jump,
    JumpIf,
    JumpIfNot,
    Match,
    AddPrefixOp,
    AddInfixOp,
    Pack,
    Unpack,
    LambdaDecl,
    ThunkDecl,
    ThunkOver,
    Neg,
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Is,
    Ceq,
    Cne,
    Clt,
    Cle,
    BNeg,
    BOr,
    BXor,
    BAnd,
    Bls,
    Brs,
    Index,
    BuildEnum,
    BuildList,
    BuildDict,
    BuildClass,
}

#[derive(Clone, Debug)]
pub(crate) enum Operand {
    None,
    Size(u64),
    String(String),
    Bytes(Vec<u8>),
    StringSize(String, u64),
    SizePair(u64, u64),
}

#[derive(Clone, Debug)]
pub(crate) struct Instruction {
    pub(crate) opcode: Opcode,
    pub(crate) operand: Operand,
}

impl Instruction {
    fn new(opcode: Opcode) -> Self {
        Self {
            opcode,
            operand: Operand::None,
        }
    }

    fn size(opcode: Opcode, value: usize) -> Self {
        Self {
            opcode,
            operand: Operand::Size(value as u64),
        }
    }

    fn string(opcode: Opcode, value: impl Into<String>) -> Self {
        Self {
            opcode,
            operand: Operand::String(value.into()),
        }
    }

    fn bytes(opcode: Opcode, value: impl Into<Vec<u8>>) -> Self {
        Self {
            opcode,
            operand: Operand::Bytes(value.into()),
        }
    }
}

#[derive(Clone, Debug)]
pub(crate) enum Constant {
    Integer(i64),
    Float(f64),
    String(Vec<u8>),
}

#[derive(Clone, Copy)]
enum Folded {
    Integer(i64),
    Bool(bool),
}

/// Accumulates the serialized instruction stream. Statements may be appended
/// one at a time so dynamically declared operators affect following input.
#[derive(Clone, Default)]
pub(crate) struct LegacyIr {
    constants: Vec<(Vec<u8>, Constant)>,
    instructions: Vec<Instruction>,
    source_mapping: BTreeMap<usize, (u64, u64)>,
    breaks: Vec<usize>,
    continues: Vec<usize>,
}

#[derive(Clone, Copy, Debug)]
pub(crate) struct InvalidConstantTag;

impl LegacyIr {
    pub(crate) fn add_statement(&mut self, statement: &Stmt) {
        self.statement(statement);
    }

    pub(crate) fn write_to(&self, path: &Path) -> io::Result<()> {
        fs::write(path, self.to_bytes())
    }

    pub(crate) fn write_debug_to(&self, path: &Path) -> io::Result<()> {
        fs::write(path, self.debug_listing())
    }

    pub(crate) fn read_from(path: &Path) -> io::Result<Result<Option<Self>, InvalidConstantTag>> {
        let bytes = fs::read(path)?;
        Ok(Self::from_bytes(&bytes))
    }

    pub(crate) fn constants(&self) -> &[(Vec<u8>, Constant)] {
        &self.constants
    }

    pub(crate) fn instructions(&self) -> &[Instruction] {
        &self.instructions
    }

    pub(crate) fn location(&self, instruction: usize) -> Location {
        self.source_mapping
            .range(..=instruction)
            .next_back()
            .map_or(Location { line: 0, column: 0 }, |(_, (line, column))| {
                Location {
                    line: *line as usize,
                    column: column.saturating_sub(1) as usize,
                }
            })
    }

    pub(crate) fn mapped_location(&self, instruction: usize) -> Option<Location> {
        self.source_mapping
            .get(&instruction)
            .map(|(line, column)| Location {
                line: *line as usize,
                column: if *line == 0 {
                    *column as usize
                } else {
                    column.saturating_sub(1) as usize
                },
            })
    }

    fn debug_listing(&self) -> Vec<u8> {
        let mut output = Vec::new();
        let constant_rows = std::iter::once(vec![b"CI".to_vec(), b"Value".to_vec()])
            .chain(
                self.constants
                    .iter()
                    .enumerate()
                    .map(|(index, (literal, _))| {
                        vec![(index + 3).to_string().into_bytes(), literal.clone()]
                    }),
            )
            .chain(std::iter::once(Vec::new()))
            .collect::<Vec<_>>();
        output.extend_from_slice(b"Constants:\n");
        render_debug_rows(&mut output, &constant_rows);

        let mut instruction_rows =
            vec![vec![b"L".to_vec(), b"Opcode".to_vec(), b"Oprand".to_vec()]];
        for (index, instruction) in self.instructions.iter().enumerate() {
            if instruction.opcode == Opcode::PlaceHolder {
                continue;
            }
            let mut row = vec![
                index.to_string().into_bytes(),
                instruction.opcode.debug_name().as_bytes().to_vec(),
            ];
            match &instruction.operand {
                Operand::None => {}
                Operand::Size(value) => row.push(value.to_string().into_bytes()),
                Operand::String(value) => row.push(symbol_to_bytes(value)),
                Operand::Bytes(value) => row.push(value.clone()),
                Operand::StringSize(value, size) => {
                    let mut rendered = b"(".to_vec();
                    rendered.extend(symbol_to_bytes(value));
                    rendered.extend(format!(", {size})").into_bytes());
                    row.push(rendered);
                }
                Operand::SizePair(first, second) => {
                    row.push(format!("({first}, {second})").into_bytes());
                }
            }
            instruction_rows.push(row);
        }
        output.extend_from_slice(b"Instructions:\n");
        render_debug_rows(&mut output, &instruction_rows);
        output
    }

    fn from_bytes(bytes: &[u8]) -> Result<Option<Self>, InvalidConstantTag> {
        let mut invalid_constant_tag = false;
        let decoded = (|| {
            let mut reader = Reader::new(bytes);
            if reader.u64()? != MAGIC {
                return None;
            }
            let constants_size = usize::try_from(reader.u64()?).ok()?;
            let instructions_size = usize::try_from(reader.u64()?).ok()?;
            let mapping_size = usize::try_from(reader.u64()?).ok()?;
            let mut constants = Vec::with_capacity(constants_size);
            for _ in 0..constants_size {
                let kind = reader.byte()?;
                let constant = match kind {
                    b'i' => {
                        let value = i64::from_ne_bytes(reader.array()?);
                        (format!("i{value}").into_bytes(), Constant::Integer(value))
                    }
                    b'f' => {
                        let value = f64::from_ne_bytes(reader.array()?);
                        (format!("f{value:.6}").into_bytes(), Constant::Float(value))
                    }
                    b's' => {
                        let value = reader.bytes()?;
                        let mut key = Vec::with_capacity(value.len() + 1);
                        key.push(b's');
                        key.extend_from_slice(&value);
                        (key, Constant::String(value))
                    }
                    _ => {
                        invalid_constant_tag = true;
                        return None;
                    }
                };
                constants.push(constant);
            }
            let mut instructions = Vec::with_capacity(instructions_size);
            for _ in 0..instructions_size {
                let opcode = Opcode::from_byte(reader.byte()?)?;
                let operand = match opcode {
                    Opcode::Pop
                    | Opcode::LoadConst
                    | Opcode::FastCall
                    | Opcode::Jump
                    | Opcode::JumpIf
                    | Opcode::JumpIfNot
                    | Opcode::Match
                    | Opcode::Unpack
                    | Opcode::ThunkDecl
                    | Opcode::BuildList
                    | Opcode::BuildDict => Operand::Size(reader.u64()?),
                    Opcode::ImportPath => Operand::Bytes(reader.bytes()?),
                    Opcode::Import
                    | Opcode::ImportPart
                    | Opcode::Load
                    | Opcode::LoadMember
                    | Opcode::StoreRef
                    | Opcode::StoreLocal
                    | Opcode::AddPrefixOp
                    | Opcode::BuildClass => Operand::String(reader.string()?),
                    Opcode::AddInfixOp => Operand::StringSize(reader.string()?, reader.u64()?),
                    Opcode::LambdaDecl => Operand::SizePair(reader.u64()?, reader.u64()?),
                    _ => Operand::None,
                };
                instructions.push(Instruction { opcode, operand });
            }
            let mut source_mapping = BTreeMap::new();
            for _ in 0..mapping_size {
                let instruction = usize::try_from(reader.u64()?).ok()?;
                source_mapping.insert(instruction, (reader.u64()?, reader.u64()?));
            }
            Some(Self {
                constants,
                instructions,
                source_mapping,
                breaks: Vec::new(),
                continues: Vec::new(),
            })
        })();
        if invalid_constant_tag {
            Err(InvalidConstantTag)
        } else {
            Ok(decoded)
        }
    }

    fn to_bytes(&self) -> Vec<u8> {
        let mut output = Vec::new();
        write_u64(&mut output, MAGIC);
        write_u64(&mut output, self.constants.len() as u64);
        write_u64(&mut output, self.instructions.len() as u64);
        write_u64(&mut output, self.source_mapping.len() as u64);

        for (_, constant) in &self.constants {
            match constant {
                Constant::Integer(value) => {
                    output.push(b'i');
                    output.extend_from_slice(&value.to_ne_bytes());
                }
                Constant::Float(value) => {
                    output.push(b'f');
                    output.extend_from_slice(&value.to_ne_bytes());
                }
                Constant::String(value) => {
                    output.push(b's');
                    write_bytes(&mut output, value);
                }
            }
        }

        for instruction in &self.instructions {
            output.push(instruction.opcode as u8);
            match &instruction.operand {
                Operand::None => {}
                Operand::Size(value) => write_u64(&mut output, *value),
                Operand::String(value) => write_string(&mut output, value),
                Operand::Bytes(value) => write_bytes(&mut output, value),
                Operand::StringSize(value, size) => {
                    write_string(&mut output, value);
                    write_u64(&mut output, *size);
                }
                Operand::SizePair(first, second) => {
                    write_u64(&mut output, *first);
                    write_u64(&mut output, *second);
                }
            }
        }

        for (instruction, (line, column)) in &self.source_mapping {
            write_u64(&mut output, *instruction as u64);
            write_u64(&mut output, *line);
            write_u64(&mut output, *column);
        }
        output
    }

    fn add(&mut self, instruction: Instruction) -> usize {
        let index = self.instructions.len();
        self.instructions.push(instruction);
        index
    }

    fn placeholder(&mut self) -> usize {
        self.add(Instruction::new(Opcode::PlaceHolder))
    }

    fn set_size(&mut self, index: usize, opcode: Opcode, value: usize) {
        self.instructions[index] = Instruction::size(opcode, value);
    }

    fn locate(&mut self, location: Location) {
        let column = if location.line == 0 {
            location.column as u64
        } else {
            location.column as u64 + 1
        };
        self.source_mapping
            .insert(self.instructions.len(), (location.line as u64, column));
    }

    fn locate_synthetic(&mut self) {
        self.source_mapping.insert(self.instructions.len(), (0, 0));
    }

    fn constant(&mut self, literal: &Literal) -> usize {
        match literal {
            Literal::None => 0,
            Literal::Bool(true) => 1,
            Literal::Bool(false) => 2,
            Literal::Integer(value) => {
                let key = format!("i{value}").into_bytes();
                self.insert_constant(key, Constant::Integer(*value))
            }
            Literal::Float(value) => {
                // `std::to_string(double)` uses fixed notation with six digits
                // after the decimal point; the key affects constant de-duplication.
                let key = format!("f{value:.6}").into_bytes();
                self.insert_constant(key, Constant::Float(*value))
            }
            Literal::String(value) => {
                let mut key = Vec::with_capacity(value.len() + 1);
                key.push(b's');
                key.extend_from_slice(value);
                self.insert_constant(key, Constant::String(value.clone()))
            }
        }
    }

    fn insert_constant(&mut self, key: Vec<u8>, value: Constant) -> usize {
        if let Some(index) = self
            .constants
            .iter()
            .position(|(candidate, _)| candidate == &key)
        {
            index + 3
        } else {
            let index = self.constants.len() + 3;
            self.constants.push((key, value));
            index
        }
    }

    fn emit_literal(&mut self, literal: &Literal) {
        let index = self.constant(literal);
        self.add(Instruction::size(Opcode::LoadConst, index));
    }

    fn expression(&mut self, expression: &Expr) {
        if let Some(folded) = fold(expression) {
            match folded {
                Folded::Integer(value) => self.emit_literal(&Literal::Integer(value)),
                Folded::Bool(value) => self.emit_literal(&Literal::Bool(value)),
            }
            return;
        }

        match expression {
            Expr::Literal(literal) => self.emit_literal(literal),
            Expr::Identifier(name, location) => {
                self.locate(*location);
                self.add(Instruction::string(Opcode::Load, name));
            }
            Expr::List(expressions) => {
                for expression in expressions.iter().rev() {
                    self.expression(expression);
                }
                self.add(Instruction::size(Opcode::BuildList, expressions.len()));
            }
            Expr::Dict(entries) => {
                for (key, value) in entries.iter().rev() {
                    self.expression(value);
                    self.expression(key);
                }
                self.add(Instruction::size(Opcode::BuildDict, entries.len()));
            }
            Expr::Lambda { parameters, body } => {
                let declaration = self.placeholder();
                for parameter in parameters {
                    if let Some(default) = &parameter.default {
                        self.expression(default);
                    }
                    let store = if parameter.by_reference {
                        Opcode::StoreRef
                    } else {
                        Opcode::StoreLocal
                    };
                    self.add(Instruction::string(store, &parameter.name));
                    if parameter.variadic {
                        let copied = self.instructions.last().cloned().expect("parameter store");
                        self.add(copied);
                        let pack = self.instructions.len() - 2;
                        self.instructions[pack] = Instruction::new(Opcode::Pack);
                    }
                }
                self.block(body);
                self.add(Instruction::new(Opcode::ReturnNone));
                self.instructions[declaration] = Instruction {
                    opcode: Opcode::LambdaDecl,
                    operand: Operand::SizePair(
                        parameters.len() as u64,
                        self.instructions.len() as u64,
                    ),
                };
            }
            Expr::Unary {
                operator,
                operand,
                location,
            } => self.unary(operator, operand, *location),
            Expr::Binary {
                left,
                operator,
                right,
                location,
            } => self.binary(left, operator, right, *location),
            Expr::Call {
                callee,
                arguments,
                location,
            } => {
                if arguments.is_empty() {
                    self.expression(callee);
                    self.locate(*location);
                    self.add(Instruction::size(Opcode::FastCall, 0));
                } else if arguments.iter().any(|argument| argument.unpack) {
                    self.add(Instruction::new(Opcode::CallAc));
                    for argument in arguments.iter().rev() {
                        self.expression(&argument.value);
                        if argument.unpack {
                            self.add(Instruction::size(Opcode::Unpack, 0));
                        }
                    }
                    self.expression(callee);
                    self.locate(*location);
                    self.add(Instruction::new(Opcode::Call));
                } else {
                    for argument in arguments.iter().rev() {
                        self.expression(&argument.value);
                    }
                    self.expression(callee);
                    self.locate(*location);
                    self.add(Instruction::size(Opcode::FastCall, arguments.len()));
                }
            }
            Expr::Member {
                object,
                name,
                location,
            } => {
                self.expression(object);
                self.locate(*location);
                self.add(Instruction::string(Opcode::LoadMember, name));
            }
            Expr::Index {
                object,
                index,
                location,
            } => {
                self.expression(index);
                self.expression(object);
                self.locate(*location);
                self.add(Instruction::new(Opcode::Index));
            }
            Expr::Conditional {
                condition,
                then_value,
                else_value,
                location,
            } => {
                self.expression(condition);
                self.locate(*location);
                let jump_if_not = self.placeholder();
                self.expression(then_value);
                let jump = self.placeholder();
                self.set_size(jump_if_not, Opcode::JumpIfNot, self.instructions.len());
                self.expression(else_value);
                self.set_size(jump, Opcode::Jump, self.instructions.len());
            }
            Expr::Delay(expression) => {
                let declaration = self.placeholder();
                self.expression(expression);
                self.add(Instruction::new(Opcode::ThunkOver));
                self.set_size(declaration, Opcode::ThunkDecl, self.instructions.len());
            }
            Expr::Enum(entries) => {
                self.add(Instruction::new(Opcode::NewScope));
                for (name, value) in entries {
                    self.emit_literal(&Literal::Integer(*value));
                    self.add(Instruction::string(Opcode::StoreRef, name));
                }
                self.add(Instruction::new(Opcode::BuildEnum));
            }
            Expr::Match {
                value,
                arms,
                key_locations,
                fallback,
            } => self.match_expression(value, arms, key_locations, fallback.as_deref()),
            Expr::Class {
                name,
                bases,
                members,
            } => {
                self.add(Instruction::new(Opcode::CallAc));
                for base in bases.iter().rev() {
                    self.expression(&base.value);
                    if base.unpack {
                        self.add(Instruction::size(Opcode::Unpack, 0));
                    }
                }
                self.add(Instruction::string(
                    Opcode::BuildClass,
                    name.as_deref().unwrap_or(""),
                ));
                for member in members {
                    self.declaration(member);
                }
                self.add(Instruction::new(Opcode::EndScope));
            }
        }
    }

    fn unary(&mut self, operator: &str, operand: &Expr, location: Location) {
        match operator {
            "-" => {
                self.expression(operand);
                self.locate(location);
                self.add(Instruction::new(Opcode::Neg));
            }
            "not" | "!" => {
                self.expression(operand);
                self.locate(location);
                let jump_if = self.placeholder();
                self.emit_literal(&Literal::Bool(true));
                let jump = self.placeholder();
                self.set_size(jump_if, Opcode::JumpIf, self.instructions.len());
                self.emit_literal(&Literal::Bool(false));
                self.set_size(jump, Opcode::Jump, self.instructions.len());
            }
            "~" => {
                self.expression(operand);
                self.locate(location);
                self.add(Instruction::new(Opcode::BNeg));
            }
            _ => {
                self.expression(operand);
                self.add(Instruction::string(Opcode::Load, operator));
                self.add(Instruction::size(Opcode::FastCall, 1));
            }
        }
    }

    fn binary(&mut self, left: &Expr, operator: &str, right: &Expr, location: Location) {
        if operator == ":" {
            self.expression(right);
            self.expression(left);
            self.add(Instruction::new(Opcode::Store));
            return;
        }

        let built_in = matches!(
            operator,
            "and"
                | "or"
                | "+"
                | "-"
                | "*"
                | "/"
                | "%"
                | "&"
                | "|"
                | "^"
                | "<<"
                | ">>"
                | "is"
                | "="
                | "!="
                | "<"
                | "<="
                | ">"
                | ">="
        );
        if !built_in {
            self.expression(right);
            self.expression(left);
            self.add(Instruction::string(Opcode::Load, operator));
            self.locate_binary(left, right, operator, location);
            self.add(Instruction::size(Opcode::FastCall, 2));
            return;
        }

        match operator {
            "and" => self.logical(left, right, location, false),
            "or" => self.logical(left, right, location, true),
            ">" => {
                self.expression(right);
                self.expression(left);
                self.locate(location);
                self.add(Instruction::new(Opcode::Cle));
            }
            ">=" => {
                self.expression(right);
                self.expression(left);
                self.locate(location);
                self.add(Instruction::new(Opcode::Clt));
            }
            _ => {
                self.expression(left);
                self.expression(right);
                let opcode = match operator {
                    "+" => Some(Opcode::Add),
                    "-" => Some(Opcode::Sub),
                    "*" => Some(Opcode::Mul),
                    "/" => Some(Opcode::Div),
                    "%" => Some(Opcode::Mod),
                    "&" => Some(Opcode::BAnd),
                    "|" => Some(Opcode::BOr),
                    "^" => Some(Opcode::BXor),
                    "<<" => Some(Opcode::Bls),
                    ">>" => Some(Opcode::Brs),
                    "is" => Some(Opcode::Is),
                    "=" => Some(Opcode::Ceq),
                    "!=" => Some(Opcode::Cne),
                    "<" => Some(Opcode::Clt),
                    "<=" => Some(Opcode::Cle),
                    _ => None,
                };
                if let Some(opcode) = opcode {
                    self.locate_binary(left, right, operator, location);
                    self.add(Instruction::new(opcode));
                } else {
                    unreachable!("all custom operators returned above")
                }
            }
        }
    }

    fn logical(&mut self, left: &Expr, right: &Expr, location: Location, is_or: bool) {
        self.expression(left);
        self.locate(location);
        let first = self.placeholder();
        self.expression(right);
        self.locate(location);
        let second = self.placeholder();
        self.emit_literal(&Literal::Bool(!is_or));
        let jump = self.placeholder();
        let other = self.instructions.len();
        self.set_size(
            first,
            if is_or {
                Opcode::JumpIf
            } else {
                Opcode::JumpIfNot
            },
            other,
        );
        self.set_size(
            second,
            if is_or {
                Opcode::JumpIf
            } else {
                Opcode::JumpIfNot
            },
            other,
        );
        self.emit_literal(&Literal::Bool(is_or));
        self.set_size(jump, Opcode::Jump, self.instructions.len());
    }

    fn locate_binary(&mut self, left: &Expr, right: &Expr, operator: &str, location: Location) {
        let parser_dropped_location = matches!(fold(left), Some(Folded::Integer(_)))
            && matches!(fold(right), Some(Folded::Integer(_)))
            && !matches!(
                operator,
                "+" | "-"
                    | "*"
                    | "/"
                    | "%"
                    | "and"
                    | "or"
                    | "is"
                    | "="
                    | "!="
                    | "<"
                    | "<="
                    | ">"
                    | ">="
            );
        if parser_dropped_location {
            self.locate_synthetic();
        } else {
            self.locate(location);
        }
    }

    fn match_expression(
        &mut self,
        value: &Expr,
        arms: &[(Vec<Expr>, Expr)],
        key_locations: &[Vec<Location>],
        fallback: Option<&Expr>,
    ) {
        self.expression(value);
        let flattened_locations: Vec<_> = key_locations.iter().flatten().copied().collect();
        let mut match_froms = vec![Vec::new(); arms.len()];
        let mut jump_froms = Vec::new();
        for (arm_index, (keys, _)) in arms.iter().enumerate() {
            for key in keys {
                self.expression(key);
                if let Some(location) = flattened_locations.get(arm_index) {
                    self.locate(*location);
                }
                match_froms[arm_index].push(self.placeholder());
            }
        }
        if let Some(fallback) = fallback {
            self.expression(fallback);
        } else {
            self.emit_literal(&Literal::None);
        }
        jump_froms.push(self.placeholder());
        for (arm_index, (_, result)) in arms.iter().enumerate() {
            for from in &match_froms[arm_index] {
                self.set_size(*from, Opcode::Match, self.instructions.len());
            }
            self.expression(result);
            jump_froms.push(self.placeholder());
        }
        let end = self.instructions.len();
        for from in jump_froms {
            self.set_size(from, Opcode::Jump, end);
        }
    }

    fn statement(&mut self, statement: &Stmt) {
        match statement {
            Stmt::Expression(expression) => {
                self.expression(expression);
                self.add(Instruction::size(Opcode::Pop, 1));
            }
            Stmt::Declaration(declaration) => self.declaration(declaration),
            Stmt::Import {
                aliases,
                from,
                import_all: _,
            } => self.import(aliases, from),
            Stmt::PrefixOperator(operator) => {
                self.add(Instruction::string(Opcode::AddPrefixOp, operator));
            }
            Stmt::InfixOperator {
                operator,
                precedence,
            } => {
                self.add(Instruction {
                    opcode: Opcode::AddInfixOp,
                    operand: Operand::StringSize(operator.clone(), *precedence),
                });
            }
            Stmt::If {
                condition,
                then_block,
                else_branch,
                location,
            } => {
                self.expression(condition);
                self.locate(*location);
                let jump_if_not = self.placeholder();
                self.block(then_block);
                if let Some(else_branch) = else_branch {
                    let jump = self.placeholder();
                    self.set_size(jump_if_not, Opcode::JumpIfNot, self.instructions.len());
                    self.statement(else_branch);
                    self.set_size(jump, Opcode::Jump, self.instructions.len());
                } else {
                    self.set_size(jump_if_not, Opcode::JumpIfNot, self.instructions.len());
                }
            }
            Stmt::Block(block) => self.block(block),
            Stmt::While {
                condition,
                body,
                location,
            } => self.while_statement(condition, body, *location),
            Stmt::DoWhile {
                body,
                condition,
                location,
            } => {
                let start = self.instructions.len();
                self.block(body);
                let condition_start = self.instructions.len();
                self.expression(condition);
                self.locate(*location);
                self.add(Instruction::size(Opcode::JumpIf, start));
                self.patch_breaks(self.instructions.len(), start);
                self.patch_continues(condition_start, start);
            }
            Stmt::Foreach {
                iterable,
                binding,
                body,
            } => self.foreach(iterable, binding.as_deref(), body),
            Stmt::Break => {
                let index = self.placeholder();
                self.breaks.push(index);
            }
            Stmt::Continue => {
                let index = self.placeholder();
                self.continues.push(index);
            }
            Stmt::Return(expressions) => {
                if expressions.is_empty() {
                    self.add(Instruction::new(Opcode::ReturnNone));
                } else {
                    for expression in expressions.iter().rev() {
                        self.expression(expression);
                    }
                    if expressions.len() > 1 {
                        self.add(Instruction::size(Opcode::BuildList, expressions.len()));
                    }
                    self.add(Instruction::new(Opcode::Return));
                }
            }
        }
    }

    fn block(&mut self, block: &Block) {
        for statement in block {
            self.statement(statement);
        }
    }

    fn declaration(&mut self, declaration: &Declaration) {
        if let [Binding::Name { name, by_reference }] = declaration.bindings.as_slice() {
            if let Some(value) = declaration.values.first() {
                self.expression(value);
            } else {
                self.emit_literal(&Literal::None);
            }
            self.store_binding(name, *by_reference);
            return;
        }

        if declaration.values.is_empty() {
            for _ in &declaration.bindings {
                self.emit_literal(&Literal::None);
            }
        } else {
            for value in declaration.values.iter().rev() {
                self.expression(value);
            }
            if declaration.values.len() == 1 && declaration.bindings.len() > 1 {
                self.add(Instruction::size(
                    Opcode::Unpack,
                    declaration.bindings.len(),
                ));
            }
        }
        for binding in &declaration.bindings {
            self.binding(binding);
        }
    }

    fn binding(&mut self, binding: &Binding) {
        match binding {
            Binding::Name { name, by_reference } => self.store_binding(name, *by_reference),
            Binding::Destructure(bindings) => {
                self.add(Instruction::size(Opcode::Unpack, bindings.len()));
                for binding in bindings {
                    self.binding(binding);
                }
            }
        }
    }

    fn store_binding(&mut self, name: &str, by_reference: bool) {
        self.add(Instruction::string(
            if by_reference {
                Opcode::StoreRef
            } else {
                Opcode::StoreLocal
            },
            name,
        ));
    }

    fn import(&mut self, aliases: &[crate::ast::ImportAlias], from: &[ModulePart]) {
        if from.is_empty() {
            for alias in aliases {
                self.module_import(&alias.module);
                self.add(Instruction::string(Opcode::StoreRef, &alias.alias));
                if alias.module.len() > 1 {
                    self.add(Instruction::size(Opcode::Pop, alias.module.len() - 1));
                }
            }
        } else if aliases.is_empty() {
            self.module_import(from);
            self.add(Instruction::new(Opcode::ImportAll));
            if from.len() > 1 {
                self.add(Instruction::size(Opcode::Pop, from.len() - 1));
            }
        } else {
            self.module_import(from);
            for alias in aliases {
                for part in &alias.module {
                    self.import_part(part);
                }
                self.add(Instruction::string(Opcode::StoreRef, &alias.alias));
                if alias.module.len() > 1 {
                    self.add(Instruction::size(Opcode::Pop, alias.module.len() - 1));
                }
            }
            self.add(Instruction::size(Opcode::Pop, from.len()));
        }
    }

    fn module_import(&mut self, module: &[ModulePart]) {
        let Some((first, remaining)) = module.split_first() else {
            return;
        };
        match first {
            ModulePart::Name(name) => {
                self.add(Instruction::string(Opcode::Import, name));
            }
            ModulePart::Path(path) => {
                self.add(Instruction::bytes(Opcode::ImportPath, path.clone()));
            }
        }
        for part in remaining {
            self.import_part(part);
        }
    }

    fn import_part(&mut self, part: &ModulePart) {
        if let ModulePart::Name(name) = part {
            self.add(Instruction::string(Opcode::ImportPart, name));
        }
    }

    fn while_statement(&mut self, condition: &Expr, body: &Block, location: Location) {
        let start = self.instructions.len();
        self.expression(condition);
        self.locate(location);
        let jump_if_not = self.placeholder();
        self.block(body);
        self.add(Instruction::size(Opcode::Jump, start));
        self.set_size(jump_if_not, Opcode::JumpIfNot, self.instructions.len());
        self.patch_breaks(self.instructions.len(), start);
        self.patch_continues(start, start);
    }

    fn foreach(&mut self, iterable: &Expr, binding: Option<&str>, body: &Block) {
        self.expression(iterable);
        self.add(Instruction::string(Opcode::LoadMember, "__iterator__"));
        self.add(Instruction::size(Opcode::FastCall, 0));
        let iterator_name = format!("//__it_{}", self.instructions.len());
        self.add(Instruction::string(Opcode::StoreRef, &iterator_name));

        let start = self.instructions.len();
        self.synthetic_call(&iterator_name, "__has_next__");
        self.locate_synthetic();
        let jump_if_not = self.placeholder();
        self.synthetic_call(&iterator_name, "__next__");
        if let Some(binding) = binding {
            self.add(Instruction::string(Opcode::StoreRef, binding));
        } else {
            self.add(Instruction::size(Opcode::Pop, 1));
        }
        self.block(body);
        self.add(Instruction::size(Opcode::Jump, start));
        self.set_size(jump_if_not, Opcode::JumpIfNot, self.instructions.len());
        self.patch_breaks(self.instructions.len(), start);
        self.patch_continues(start, start);
    }

    fn synthetic_call(&mut self, object: &str, member: &str) {
        self.locate_synthetic();
        self.add(Instruction::string(Opcode::Load, object));
        self.locate_synthetic();
        self.add(Instruction::string(Opcode::LoadMember, member));
        self.locate_synthetic();
        self.add(Instruction::size(Opcode::FastCall, 0));
    }

    fn patch_breaks(&mut self, target: usize, base: usize) {
        let mut remaining = Vec::new();
        for index in std::mem::take(&mut self.breaks) {
            if index > base {
                self.set_size(index, Opcode::Jump, target);
            } else {
                remaining.push(index);
            }
        }
        self.breaks = remaining;
    }

    fn patch_continues(&mut self, target: usize, base: usize) {
        let mut remaining = Vec::new();
        for index in std::mem::take(&mut self.continues) {
            if index > base {
                self.set_size(index, Opcode::Jump, target);
            } else {
                remaining.push(index);
            }
        }
        self.continues = remaining;
    }
}

impl Opcode {
    fn debug_name(self) -> &'static str {
        match self {
            Self::PlaceHolder => "PlaceHolder",
            Self::Pop => "Pop",
            Self::Import => "Import",
            Self::ImportPath => "ImportPath",
            Self::ImportAll => "ImportAll",
            Self::ImportPart => "ImportPart",
            Self::Load => "Load",
            Self::LoadConst => "LoadConst",
            Self::LoadMember => "LoadMember",
            Self::Store => "Store",
            Self::StoreRef => "StoreRef",
            Self::StoreLocal => "StoreLocal",
            Self::NewScope => "NewScope",
            Self::EndScope => "EndScope",
            Self::CallAc => "CallAc",
            Self::Call => "Call",
            Self::FastCall => "FastCall",
            Self::Return => "Return",
            Self::ReturnNone => "ReturnNone",
            Self::Jump => "Jump",
            Self::JumpIf => "JumpIf",
            Self::JumpIfNot => "JumpIfNot",
            Self::Match => "Match",
            Self::AddPrefixOp => "AddPrefixOp",
            Self::AddInfixOp => "AddInfixOp",
            Self::Pack => "Pack",
            Self::Unpack => "Unpack",
            Self::LambdaDecl => "LambdaDecl",
            Self::ThunkDecl => "ThunkDecl",
            Self::ThunkOver => "ThunkOver",
            Self::Neg => "Neg",
            Self::Add => "Add",
            Self::Sub => "Sub",
            Self::Mul => "Mul",
            Self::Div => "Div",
            Self::Mod => "Mod",
            Self::Is => "Is",
            Self::Ceq => "CEQ",
            Self::Cne => "CNE",
            Self::Clt => "CLT",
            Self::Cle => "CLE",
            Self::BNeg => "BNeg",
            Self::BOr => "BOr",
            Self::BXor => "BXor",
            Self::BAnd => "BAnd",
            Self::Bls => "BLS",
            Self::Brs => "BRS",
            Self::Index => "Index",
            Self::BuildEnum => "BuildEnum",
            Self::BuildList => "BuildList",
            Self::BuildDict => "BuildDict",
            Self::BuildClass => "BuildClass",
        }
    }

    fn from_byte(value: u8) -> Option<Self> {
        Some(match value {
            0 => Self::PlaceHolder,
            1 => Self::Pop,
            2 => Self::Import,
            3 => Self::ImportPath,
            4 => Self::ImportAll,
            5 => Self::ImportPart,
            6 => Self::Load,
            7 => Self::LoadConst,
            8 => Self::LoadMember,
            9 => Self::Store,
            10 => Self::StoreRef,
            11 => Self::StoreLocal,
            12 => Self::NewScope,
            13 => Self::EndScope,
            14 => Self::CallAc,
            15 => Self::Call,
            16 => Self::FastCall,
            17 => Self::Return,
            18 => Self::ReturnNone,
            19 => Self::Jump,
            20 => Self::JumpIf,
            21 => Self::JumpIfNot,
            22 => Self::Match,
            23 => Self::AddPrefixOp,
            24 => Self::AddInfixOp,
            25 => Self::Pack,
            26 => Self::Unpack,
            27 => Self::LambdaDecl,
            28 => Self::ThunkDecl,
            29 => Self::ThunkOver,
            30 => Self::Neg,
            31 => Self::Add,
            32 => Self::Sub,
            33 => Self::Mul,
            34 => Self::Div,
            35 => Self::Mod,
            36 => Self::Is,
            37 => Self::Ceq,
            38 => Self::Cne,
            39 => Self::Clt,
            40 => Self::Cle,
            41 => Self::BNeg,
            42 => Self::BOr,
            43 => Self::BXor,
            44 => Self::BAnd,
            45 => Self::Bls,
            46 => Self::Brs,
            47 => Self::Index,
            48 => Self::BuildEnum,
            49 => Self::BuildList,
            50 => Self::BuildDict,
            51 => Self::BuildClass,
            _ => return None,
        })
    }
}

fn render_debug_rows(output: &mut Vec<u8>, rows: &[Vec<Vec<u8>>]) {
    let mut widths = Vec::new();
    for row in rows {
        for (index, value) in row.iter().enumerate() {
            if widths.len() <= index {
                widths.push(0);
            }
            let width = (value.len() / 4 + 1) * 4;
            widths[index] = widths[index].max(width);
        }
    }
    for row in rows {
        for (index, value) in row.iter().enumerate() {
            output.extend_from_slice(value);
            if index + 1 < row.len() {
                output.extend(std::iter::repeat_n(b' ', widths[index] - value.len()));
            }
        }
        output.push(b'\n');
    }
}

struct Reader<'a> {
    bytes: &'a [u8],
    offset: usize,
}

impl<'a> Reader<'a> {
    fn new(bytes: &'a [u8]) -> Self {
        Self { bytes, offset: 0 }
    }

    fn byte(&mut self) -> Option<u8> {
        let byte = *self.bytes.get(self.offset)?;
        self.offset += 1;
        Some(byte)
    }

    fn array<const N: usize>(&mut self) -> Option<[u8; N]> {
        let end = self.offset.checked_add(N)?;
        let value = self.bytes.get(self.offset..end)?.try_into().ok()?;
        self.offset = end;
        Some(value)
    }

    fn u64(&mut self) -> Option<u64> {
        Some(u64::from_ne_bytes(self.array()?))
    }

    fn bytes(&mut self) -> Option<Vec<u8>> {
        let size = usize::try_from(self.u64()?).ok()?;
        let end = self.offset.checked_add(size)?;
        let value = self.bytes.get(self.offset..end)?.to_vec();
        self.offset = end;
        Some(value)
    }

    fn string(&mut self) -> Option<String> {
        Some(symbol_from_bytes(&self.bytes()?))
    }
}

fn fold(expression: &Expr) -> Option<Folded> {
    match expression {
        Expr::Literal(Literal::Integer(value)) => Some(Folded::Integer(*value)),
        Expr::Binary {
            left,
            operator,
            right,
            ..
        } => {
            let (Folded::Integer(left), Folded::Integer(right)) = (fold(left)?, fold(right)?)
            else {
                return None;
            };
            match operator.as_str() {
                "+" => left.checked_add(right).map(Folded::Integer),
                "-" => left.checked_sub(right).map(Folded::Integer),
                "*" => left.checked_mul(right).map(Folded::Integer),
                "/" if right != 0 => left.checked_div(right).map(Folded::Integer),
                "%" if right != 0 => left.checked_rem(right).map(Folded::Integer),
                "and" => Some(Folded::Bool(left != 0 && right != 0)),
                "or" => Some(Folded::Bool(left != 0 || right != 0)),
                "is" | "=" => Some(Folded::Bool(left == right)),
                "!=" => Some(Folded::Bool(left != right)),
                "<" => Some(Folded::Bool(left < right)),
                "<=" => Some(Folded::Bool(left <= right)),
                ">" => Some(Folded::Bool(left > right)),
                ">=" => Some(Folded::Bool(left >= right)),
                _ => None,
            }
        }
        _ => None,
    }
}

fn write_u64(output: &mut Vec<u8>, value: u64) {
    output.extend_from_slice(&value.to_ne_bytes());
}

fn write_string(output: &mut Vec<u8>, value: &str) {
    write_bytes(output, &symbol_to_bytes(value));
}

fn write_bytes(output: &mut Vec<u8>, value: &[u8]) {
    write_u64(output, value.len() as u64);
    output.extend_from_slice(value);
}
