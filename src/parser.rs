use std::collections::{HashMap, HashSet};
use std::fmt;

use crate::ast::{
    Argument, Binding, Block, Declaration, Expr, ImportAlias, Literal, ModulePart, Parameter, Stmt,
};
use crate::{LexError, Lexer, Location, Token, TokenKind};

#[derive(Clone, Debug, PartialEq)]
pub struct ParseError {
    pub location: Location,
    pub message: String,
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "{}:{}: error: {}",
            self.location.line, self.location.column, self.message
        )
    }
}

impl std::error::Error for ParseError {}

impl From<LexError> for ParseError {
    fn from(error: LexError) -> Self {
        Self {
            location: error.location,
            message: error.message,
        }
    }
}

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
    prefix_operators: HashSet<String>,
    infix_operators: HashMap<String, u16>,
}

impl Parser {
    pub fn new(source: &str, name: impl Into<String>) -> Result<Self, ParseError> {
        let tokens = Lexer::new(source, name).tokenize()?;
        let prefix_operators = ["not", "!", "-", "~"]
            .into_iter()
            .map(str::to_owned)
            .collect();
        let infix_operators = [
            ("or", 100),
            ("and", 110),
            ("|", 120),
            ("^", 130),
            ("&", 140),
            ("=", 150),
            ("!=", 150),
            ("<", 160),
            ("<=", 160),
            (">", 160),
            (">=", 160),
            ("<<", 170),
            (">>", 170),
            ("+", 180),
            ("-", 180),
            ("is", 190),
            ("*", 190),
            ("/", 190),
            ("%", 190),
        ]
        .into_iter()
        .map(|(operator, precedence)| (operator.to_owned(), precedence))
        .collect();
        Ok(Self {
            tokens,
            current: 0,
            prefix_operators,
            infix_operators,
        })
    }

    pub fn parse(mut self) -> Result<Block, ParseError> {
        let mut statements = Vec::new();
        self.skip_semicolons();
        while !self.at(TokenKind::End) {
            statements.push(self.statement()?);
            self.skip_semicolons();
        }
        Ok(statements)
    }

    pub fn parse_next(&mut self) -> Result<Option<Stmt>, ParseError> {
        self.skip_semicolons();
        if self.at(TokenKind::End) {
            return Ok(None);
        }
        let statement = self.statement()?;
        self.skip_semicolons();
        Ok(Some(statement))
    }

    pub fn add_prefix_operator(&mut self, operator: impl Into<String>) {
        self.prefix_operators.insert(operator.into());
    }

    pub fn add_infix_operator(&mut self, operator: impl Into<String>, precedence: u16) {
        self.infix_operators.insert(operator.into(), precedence);
    }

    fn statement(&mut self) -> Result<Stmt, ParseError> {
        match self.peek().kind {
            TokenKind::At => {
                self.advance();
                if matches!(
                    self.peek().kind,
                    TokenKind::LeftParen | TokenKind::Colon | TokenKind::LeftBrace
                ) {
                    Ok(Stmt::Expression(self.lambda()?))
                } else {
                    Ok(Stmt::Declaration(self.declaration()?))
                }
            }
            TokenKind::Class => self.class_declaration(),
            TokenKind::Use => self.import(),
            TokenKind::PrefixOp => self.prefix_operator(),
            TokenKind::InfixOp => self.infix_operator(),
            TokenKind::If | TokenKind::Elif => self.if_statement(),
            TokenKind::While => self.while_statement(),
            TokenKind::Do => self.do_while_statement(),
            TokenKind::Foreach => self.foreach_statement(),
            TokenKind::Break => {
                self.advance();
                Ok(Stmt::Break)
            }
            TokenKind::Continue => {
                self.advance();
                Ok(Stmt::Continue)
            }
            TokenKind::Return => self.return_statement(),
            _ => Ok(Stmt::Expression(self.expression()?)),
        }
    }

    fn declaration(&mut self) -> Result<Declaration, ParseError> {
        let first = self.binding()?;
        let mut bindings = vec![first];
        while self.take(TokenKind::Comma).is_some() {
            bindings.push(self.binding()?);
        }

        if bindings.len() == 1
            && let Binding::Name {
                name,
                by_reference: false,
            } = &bindings[0]
            && self.take(TokenKind::LeftParen).is_some()
        {
            let name = name.clone();
            let parameters = self.parameters_after_left_paren()?;
            let body = self.function_body()?;
            return Ok(Declaration {
                bindings: vec![Binding::Name {
                    name,
                    by_reference: true,
                }],
                values: vec![Expr::Lambda { parameters, body }],
            });
        }

        let values = if self.take(TokenKind::Colon).is_some() {
            self.expression_list()?
        } else {
            Vec::new()
        };
        Ok(Declaration { bindings, values })
    }

    fn binding(&mut self) -> Result<Binding, ParseError> {
        if self.take(TokenKind::LeftBracket).is_some() {
            let mut bindings = vec![self.binding()?];
            while self.take(TokenKind::Comma).is_some() {
                bindings.push(self.binding()?);
            }
            self.expect(TokenKind::RightBracket, "expected ']'")?;
            return Ok(Binding::Destructure(bindings));
        }
        let by_reference = self.take(TokenKind::BitAnd).is_some();
        let name = self.identifier("expected an identifier")?;
        Ok(Binding::Name { name, by_reference })
    }

    fn class_declaration(&mut self) -> Result<Stmt, ParseError> {
        let class = self.class_expression()?;
        let Expr::Class {
            name: Some(name), ..
        } = &class
        else {
            return Err(self.error("expected class name"));
        };
        Ok(Stmt::Declaration(Declaration {
            bindings: vec![Binding::Name {
                name: name.clone(),
                by_reference: true,
            }],
            values: vec![class],
        }))
    }

    fn import(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        if self.take(TokenKind::Multiply).is_some() {
            self.expect(TokenKind::From, "need a module here")?;
            return Ok(Stmt::Import {
                aliases: Vec::new(),
                from: self.nested_module()?,
                import_all: true,
            });
        }
        let mut aliases = vec![self.import_alias()?];
        while self.take(TokenKind::Comma).is_some() {
            aliases.push(self.import_alias()?);
        }
        let from = if self.take(TokenKind::From).is_some() {
            self.nested_module()?
        } else {
            Vec::new()
        };
        Ok(Stmt::Import {
            aliases,
            from,
            import_all: false,
        })
    }

    fn import_alias(&mut self) -> Result<ImportAlias, ParseError> {
        let module = self.nested_module()?;
        let alias = if self.take(TokenKind::As).is_some() {
            self.identifier("need the alias here")?
        } else {
            match module.last() {
                Some(ModulePart::Name(name)) => name.clone(),
                Some(ModulePart::Path(_)) => {
                    return Err(self.error("use direct path of module need an alias"));
                }
                None => return Err(self.error("expect a module here")),
            }
        };
        Ok(ImportAlias { module, alias })
    }

    fn nested_module(&mut self) -> Result<Vec<ModulePart>, ParseError> {
        let mut parts = vec![self.module_part()?];
        while self.take(TokenKind::Dot).is_some() {
            let part = self.module_part()?;
            if matches!(part, ModulePart::Path(_)) {
                return Err(self.error("module denoted by path must be the top module"));
            }
            parts.push(part);
        }
        Ok(parts)
    }

    fn module_part(&mut self) -> Result<ModulePart, ParseError> {
        let token = self.advance().clone();
        match token.kind {
            TokenKind::Identifier => Ok(ModulePart::Name(token.lexeme)),
            TokenKind::String => Ok(ModulePart::Path(token.lexeme)),
            _ => Err(ParseError {
                location: token.location,
                message: "expect a module here".to_owned(),
            }),
        }
    }

    fn prefix_operator(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let operator = self.identifier("expected an identifier here")?;
        self.prefix_operators.insert(operator.clone());
        Ok(Stmt::PrefixOperator(operator))
    }

    fn infix_operator(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let precedence = if self.at(TokenKind::Integer) {
            let token = self.advance().clone();
            token
                .lexeme
                .parse()
                .map_err(|_| self.error_at(token.location, "invalid precedence"))?
        } else {
            50
        };
        let operator = self.identifier("expected an identifier here")?;
        self.infix_operators.insert(operator.clone(), precedence);
        Ok(Stmt::InfixOperator {
            operator,
            precedence,
        })
    }

    fn if_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let condition = self.expression()?;
        let then_block = self.block()?;
        let else_branch = if self.at(TokenKind::Elif) {
            Some(Box::new(self.if_statement()?))
        } else if self.take(TokenKind::Else).is_some() {
            Some(Box::new(Stmt::Block(self.block()?)))
        } else {
            None
        };
        Ok(Stmt::If {
            condition,
            then_block,
            else_branch,
        })
    }

    fn while_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let condition = self.expression()?;
        let body = self.block()?;
        Ok(Stmt::While { condition, body })
    }

    fn do_while_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let body = self.block()?;
        self.expect(TokenKind::While, "expected keyword 'while' after 'do'")?;
        let condition = self.expression()?;
        Ok(Stmt::DoWhile { body, condition })
    }

    fn foreach_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let iterable = self.expression()?;
        let binding = if self.take(TokenKind::As).is_some() {
            Some(self.identifier("expected an identifier here")?)
        } else {
            None
        };
        let body = self.block()?;
        Ok(Stmt::Foreach {
            iterable,
            binding,
            body,
        })
    }

    fn return_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        if self.at(TokenKind::Semicolon) || self.at(TokenKind::RightBrace) {
            return Ok(Stmt::Return(Vec::new()));
        }
        Ok(Stmt::Return(self.expression_list()?))
    }

    fn expression_list(&mut self) -> Result<Vec<Expr>, ParseError> {
        let mut expressions = vec![self.delay_expression()?];
        while self.take(TokenKind::Comma).is_some() {
            expressions.push(self.delay_expression()?);
        }
        Ok(expressions)
    }

    fn delay_expression(&mut self) -> Result<Expr, ParseError> {
        if self.take(TokenKind::Delay).is_some() {
            Ok(Expr::Delay(Box::new(self.expression()?)))
        } else {
            self.expression()
        }
    }

    fn expression(&mut self) -> Result<Expr, ParseError> {
        let expression = self.binary_expression(0)?;
        if let Some(question) = self.take(TokenKind::Question) {
            let then_value = self.expression()?;
            self.expect(TokenKind::Comma, "expected ',' here")?;
            let else_value = self.expression()?;
            Ok(Expr::Conditional {
                condition: Box::new(expression),
                then_value: Box::new(then_value),
                else_value: Box::new(else_value),
                location: question.location,
            })
        } else {
            Ok(expression)
        }
    }

    fn binary_expression(&mut self, minimum_precedence: u16) -> Result<Expr, ParseError> {
        let mut left = self.prefix_expression()?;
        loop {
            if self.at(TokenKind::Colon) {
                if minimum_precedence > 1 {
                    break;
                }
                let operator = self.advance().clone();
                let right = self.binary_expression(1)?;
                left = Expr::Binary {
                    left: Box::new(left),
                    operator: ":".to_owned(),
                    right: Box::new(right),
                    location: operator.location,
                };
                continue;
            }
            let Some(precedence) = self.infix_operators.get(&self.peek().lexeme).copied() else {
                break;
            };
            if precedence < minimum_precedence {
                break;
            }
            let operator = self.advance().clone();
            let right = self.binary_expression(precedence + 1)?;
            left = Expr::Binary {
                left: Box::new(left),
                operator: operator.lexeme,
                right: Box::new(right),
                location: operator.location,
            };
        }
        Ok(left)
    }

    fn prefix_expression(&mut self) -> Result<Expr, ParseError> {
        if self.prefix_operators.contains(&self.peek().lexeme) {
            let operator = self.advance().clone();
            let operand = self.prefix_expression()?;
            return Ok(Expr::Unary {
                operator: operator.lexeme,
                operand: Box::new(operand),
                location: operator.location,
            });
        }
        let primary = self.primary()?;
        self.postfix(primary)
    }

    fn primary(&mut self) -> Result<Expr, ParseError> {
        let token = self.advance().clone();
        match token.kind {
            TokenKind::None => Ok(Expr::Literal(Literal::None)),
            TokenKind::True => Ok(Expr::Literal(Literal::Bool(true))),
            TokenKind::False => Ok(Expr::Literal(Literal::Bool(false))),
            TokenKind::Integer => token
                .lexeme
                .parse()
                .map(|value| Expr::Literal(Literal::Integer(value)))
                .map_err(|_| self.error_at(token.location, "invalid integer")),
            TokenKind::Float => token
                .lexeme
                .parse()
                .map(|value| Expr::Literal(Literal::Float(value)))
                .map_err(|_| self.error_at(token.location, "invalid float")),
            TokenKind::String => Ok(Expr::Literal(Literal::String(token.lexeme))),
            TokenKind::Identifier => Ok(Expr::Identifier(token.lexeme, token.location)),
            TokenKind::LeftParen => {
                let expression = self.expression()?;
                self.expect(TokenKind::RightParen, "expected ')' here")?;
                Ok(expression)
            }
            TokenKind::At => self.lambda(),
            TokenKind::LeftBracket => self.list(),
            TokenKind::LeftBrace => {
                self.current -= 1;
                Ok(Expr::Call {
                    callee: Box::new(Expr::Lambda {
                        parameters: Vec::new(),
                        body: self.block()?,
                    }),
                    arguments: Vec::new(),
                    location: token.location,
                })
            }
            TokenKind::Dict => self.dict(),
            TokenKind::Enum => self.enum_expression(),
            TokenKind::Match => self.match_expression(),
            TokenKind::Class => {
                self.current -= 1;
                self.class_expression()
            }
            _ => Err(self.error_at(token.location, "expected an expr here")),
        }
    }

    fn postfix(&mut self, mut expression: Expr) -> Result<Expr, ParseError> {
        loop {
            if let Some(dot) = self.take(TokenKind::Dot) {
                expression = Expr::Member {
                    object: Box::new(expression),
                    name: self.identifier("expect an identifier here")?,
                    location: dot.location,
                };
            } else if let Some(left_paren) = self.take(TokenKind::LeftParen) {
                expression = Expr::Call {
                    callee: Box::new(expression),
                    arguments: self.arguments_after_left_paren()?,
                    location: left_paren.location,
                };
            } else if let Some(left_bracket) = self.take(TokenKind::LeftBracket) {
                let index = self.expression()?;
                self.expect(TokenKind::RightBracket, "expected ']'")?;
                expression = Expr::Index {
                    object: Box::new(expression),
                    index: Box::new(index),
                    location: left_bracket.location,
                };
            } else {
                break;
            }
        }
        Ok(expression)
    }

    fn lambda(&mut self) -> Result<Expr, ParseError> {
        let parameters = if self.take(TokenKind::LeftParen).is_some() {
            self.parameters_after_left_paren()?
        } else {
            Vec::new()
        };
        Ok(Expr::Lambda {
            parameters,
            body: self.function_body()?,
        })
    }

    fn parameters_after_left_paren(&mut self) -> Result<Vec<Parameter>, ParseError> {
        let mut parameters = Vec::new();
        let mut need_default = false;
        while !self.at(TokenKind::RightParen) {
            let variadic = self.take(TokenKind::Ellipsis).is_some();
            let by_reference = self.take(TokenKind::BitAnd).is_some();
            let name = self.identifier("expected an identifier here")?;
            let default = if self.take(TokenKind::Colon).is_some() {
                need_default = true;
                Some(self.expression()?)
            } else {
                if need_default && !variadic {
                    return Err(self.error(
                        "parameter without default argument cannot follow parameter with default argument",
                    ));
                }
                None
            };
            parameters.push(Parameter {
                name,
                by_reference,
                variadic,
                default,
            });
            if self.take(TokenKind::Comma).is_none() {
                break;
            }
            if variadic {
                return Err(self.error("packed parameter should be the last parameter"));
            }
        }
        self.expect(TokenKind::RightParen, "expected ')' here")?;
        Ok(parameters)
    }

    fn function_body(&mut self) -> Result<Block, ParseError> {
        if self.take(TokenKind::Colon).is_some() {
            Ok(vec![Stmt::Return(self.expression_list()?)])
        } else {
            self.block()
        }
    }

    fn arguments_after_left_paren(&mut self) -> Result<Vec<Argument>, ParseError> {
        let mut arguments = Vec::new();
        while !self.at(TokenKind::RightParen) {
            let value = self.delay_expression()?;
            let unpack = self.take(TokenKind::Ellipsis).is_some();
            arguments.push(Argument { value, unpack });
            if self.take(TokenKind::Comma).is_none() {
                break;
            }
        }
        self.expect(TokenKind::RightParen, "expected ')'")?;
        Ok(arguments)
    }

    fn block(&mut self) -> Result<Block, ParseError> {
        self.expect(TokenKind::LeftBrace, "expect '{'")?;
        let mut statements = Vec::new();
        self.skip_semicolons();
        while !self.at(TokenKind::RightBrace) {
            if self.at(TokenKind::End) {
                return Err(self.error("expected '}'"));
            }
            statements.push(self.statement()?);
            self.skip_semicolons();
        }
        self.advance();
        Ok(statements)
    }

    fn list(&mut self) -> Result<Expr, ParseError> {
        let mut values = Vec::new();
        while !self.at(TokenKind::RightBracket) {
            values.push(self.expression()?);
            if self.take(TokenKind::Comma).is_none() {
                break;
            }
        }
        self.expect(TokenKind::RightBracket, "expected ']'")?;
        Ok(Expr::List(values))
    }

    fn dict(&mut self) -> Result<Expr, ParseError> {
        self.expect(TokenKind::LeftBrace, "expected '{'")?;
        let mut entries = Vec::new();
        while !self.at(TokenKind::RightBrace) {
            let key = self.expression()?;
            self.expect(TokenKind::FatArrow, "expected '=>'")?;
            let value = self.expression()?;
            entries.push((key, value));
            if self.take(TokenKind::Comma).is_none() {
                break;
            }
        }
        self.expect(TokenKind::RightBrace, "expected '}'")?;
        Ok(Expr::Dict(entries))
    }

    fn enum_expression(&mut self) -> Result<Expr, ParseError> {
        self.expect(TokenKind::LeftBrace, "expected '{' here")?;
        let mut values = Vec::new();
        let mut next_value = 0_i64;
        while !self.at(TokenKind::RightBrace) {
            let name = self.identifier("expected identifier here")?;
            if self.take(TokenKind::Colon).is_some() {
                let token = self.expect(TokenKind::Integer, "expected integer here")?;
                next_value = token
                    .lexeme
                    .parse()
                    .map_err(|_| self.error_at(token.location, "invalid integer"))?;
            }
            values.push((name, next_value));
            next_value += 1;
            if self.take(TokenKind::Comma).is_none() {
                break;
            }
        }
        self.expect(TokenKind::RightBrace, "expected '}'")?;
        Ok(Expr::Enum(values))
    }

    fn match_expression(&mut self) -> Result<Expr, ParseError> {
        let value = self.expression()?;
        self.expect(TokenKind::LeftBrace, "expected '{'")?;
        let mut arms = Vec::new();
        let mut fallback = None;
        while !self.at(TokenKind::RightBrace) {
            if self.take(TokenKind::FatArrow).is_some() {
                fallback = Some(Box::new(self.expression()?));
            } else {
                let mut keys = vec![self.expression()?];
                while self.take(TokenKind::Comma).is_some() && !self.at(TokenKind::FatArrow) {
                    keys.push(self.expression()?);
                }
                self.expect(TokenKind::FatArrow, "expected symbol '=>'")?;
                arms.push((keys, self.expression()?));
            }
            self.take(TokenKind::Comma);
        }
        self.advance();
        Ok(Expr::Match {
            value: Box::new(value),
            arms,
            fallback,
        })
    }

    fn class_expression(&mut self) -> Result<Expr, ParseError> {
        self.expect(TokenKind::Class, "expected 'class'")?;
        let bases = if self.take(TokenKind::LeftParen).is_some() {
            self.arguments_after_left_paren()?
        } else {
            Vec::new()
        };
        let name = if self.at(TokenKind::Identifier) {
            Some(self.identifier("expected class name")?)
        } else {
            None
        };
        self.expect(TokenKind::LeftBrace, "expected '{'")?;
        let mut members = Vec::new();
        self.skip_semicolons();
        while !self.at(TokenKind::RightBrace) {
            self.take(TokenKind::At);
            members.push(self.declaration()?);
            self.skip_semicolons();
        }
        self.advance();
        Ok(Expr::Class {
            name,
            bases,
            members,
        })
    }

    fn identifier(&mut self, message: &str) -> Result<String, ParseError> {
        let token = self.expect(TokenKind::Identifier, message)?;
        Ok(token.lexeme)
    }

    fn skip_semicolons(&mut self) {
        while self.take(TokenKind::Semicolon).is_some() {}
    }

    fn at(&self, kind: TokenKind) -> bool {
        self.peek().kind == kind
    }

    fn peek(&self) -> &Token {
        &self.tokens[self.current]
    }

    fn advance(&mut self) -> &Token {
        let token = &self.tokens[self.current];
        if token.kind != TokenKind::End {
            self.current += 1;
        }
        token
    }

    fn take(&mut self, kind: TokenKind) -> Option<Token> {
        if self.at(kind) {
            Some(self.advance().clone())
        } else {
            None
        }
    }

    fn expect(&mut self, kind: TokenKind, message: &str) -> Result<Token, ParseError> {
        if self.at(kind) {
            Ok(self.advance().clone())
        } else {
            Err(self.error(message))
        }
    }

    fn error(&self, message: impl Into<String>) -> ParseError {
        self.error_at(self.peek().location, message)
    }

    fn error_at(&self, location: Location, message: impl Into<String>) -> ParseError {
        ParseError {
            location,
            message: message.into(),
        }
    }
}
