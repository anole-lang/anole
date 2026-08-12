//! Anole language implementation.

pub const VERSION_LITERAL: &str = "0.0.25 2026/08/12";

pub mod ast;
mod ir;
pub mod lexer;
pub mod parser;
pub mod runtime;

pub use lexer::{LexError, Lexer, Location, Token, TokenKind};
pub use parser::{ParseError, Parser};
pub use runtime::{Interpreter, RuntimeError};
