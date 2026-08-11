//! Anole language implementation.

pub const VERSION_LITERAL: &str = "0.0.24 2021/12/12";

pub mod ast;
pub mod lexer;
pub mod parser;
pub mod runtime;

pub use lexer::{LexError, Lexer, Location, Token, TokenKind};
pub use parser::{ParseError, Parser};
pub use runtime::{Interpreter, RuntimeError};
