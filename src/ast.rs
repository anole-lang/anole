use crate::Location;

#[derive(Clone, Debug, PartialEq)]
pub enum Literal {
    None,
    Bool(bool),
    Integer(i64),
    Float(f64),
    String(String),
}

#[derive(Clone, Debug, PartialEq)]
pub struct Parameter {
    pub name: String,
    pub by_reference: bool,
    pub variadic: bool,
    pub default: Option<Expr>,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Argument {
    pub value: Expr,
    pub unpack: bool,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Expr {
    Literal(Literal),
    Identifier(String, Location),
    List(Vec<Expr>),
    Dict(Vec<(Expr, Expr)>),
    Lambda {
        parameters: Vec<Parameter>,
        body: Block,
    },
    Unary {
        operator: String,
        operand: Box<Expr>,
        location: Location,
    },
    Binary {
        left: Box<Expr>,
        operator: String,
        right: Box<Expr>,
        location: Location,
    },
    Call {
        callee: Box<Expr>,
        arguments: Vec<Argument>,
        location: Location,
    },
    Member {
        object: Box<Expr>,
        name: String,
        location: Location,
    },
    Index {
        object: Box<Expr>,
        index: Box<Expr>,
        location: Location,
    },
    Conditional {
        condition: Box<Expr>,
        then_value: Box<Expr>,
        else_value: Box<Expr>,
        location: Location,
    },
    Delay(Box<Expr>),
    Enum(Vec<(String, i64)>),
    Match {
        value: Box<Expr>,
        arms: Vec<(Vec<Expr>, Expr)>,
        fallback: Option<Box<Expr>>,
    },
    Class {
        name: Option<String>,
        bases: Vec<Argument>,
        members: Vec<Declaration>,
    },
}

#[derive(Clone, Debug, PartialEq)]
pub enum Binding {
    Name { name: String, by_reference: bool },
    Destructure(Vec<Binding>),
}

#[derive(Clone, Debug, PartialEq)]
pub struct Declaration {
    pub bindings: Vec<Binding>,
    pub values: Vec<Expr>,
}

#[derive(Clone, Debug, PartialEq)]
pub enum ModulePart {
    Name(String),
    Path(String),
}

#[derive(Clone, Debug, PartialEq)]
pub struct ImportAlias {
    pub module: Vec<ModulePart>,
    pub alias: String,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Stmt {
    Expression(Expr),
    Declaration(Declaration),
    Import {
        aliases: Vec<ImportAlias>,
        from: Vec<ModulePart>,
        import_all: bool,
    },
    PrefixOperator(String),
    InfixOperator {
        operator: String,
        precedence: u16,
    },
    If {
        condition: Expr,
        then_block: Block,
        else_branch: Option<Box<Stmt>>,
    },
    Block(Block),
    While {
        condition: Expr,
        body: Block,
    },
    DoWhile {
        body: Block,
        condition: Expr,
    },
    Foreach {
        iterable: Expr,
        binding: Option<String>,
        body: Block,
    },
    Break,
    Continue,
    Return(Vec<Expr>),
}

pub type Block = Vec<Stmt>;
