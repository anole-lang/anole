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
    context: Option<Box<ParseErrorContext>>,
}

#[derive(Clone, Debug, PartialEq)]
struct ParseErrorContext {
    name: String,
    line: String,
    name_bytes: Vec<u8>,
    line_bytes: Vec<u8>,
}

impl ParseError {
    pub(crate) fn render_bytes(&self) -> Vec<u8> {
        let Some(context) = &self.context else {
            return self.message.as_bytes().to_vec();
        };
        let column = self.location.column + 1;
        let mut rendered = b"\x1b[1m".to_vec();
        rendered.extend(&context.name_bytes);
        rendered.extend(
            format!(
                ":{}:{column}: \x1b[0m\x1b[31merror: \x1b[0m{}\n",
                self.location.line, self.message
            )
            .as_bytes(),
        );
        rendered.extend(&context.line_bytes);
        rendered.push(b'\n');
        rendered.extend(vec![b' '; column.saturating_sub(1)]);
        rendered.extend(b"\x1b[31m^\x1b[0m");
        rendered
    }
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let Some(context) = &self.context else {
            return write!(f, "{}", self.message);
        };
        let column = self.location.column + 1;
        write!(
            f,
            "\x1b[1m{}:{}:{}: \x1b[0m\x1b[31merror: \x1b[0m{}\n{}\n{}\x1b[31m^\x1b[0m",
            context.name,
            self.location.line,
            column,
            self.message,
            context.line,
            " ".repeat(column.saturating_sub(1)),
        )
    }
}

impl std::error::Error for ParseError {}

impl From<LexError> for ParseError {
    fn from(error: LexError) -> Self {
        Self {
            location: error.location,
            message: error.message,
            context: None,
        }
    }
}

pub struct Parser {
    tokens: Vec<Token>,
    current: usize,
    prefix_operators: HashSet<String>,
    infix_operators: HashMap<String, u128>,
    custom_infix_count: u64,
    frozen_lookahead_operator: Option<(usize, bool, Option<u128>)>,
    source_bytes: Vec<u8>,
    name: String,
}

impl Parser {
    pub fn new(source: &str, name: impl Into<String>) -> Result<Self, ParseError> {
        Self::new_bytes(source.as_bytes(), name)
    }

    pub fn new_bytes(source: &[u8], name: impl Into<String>) -> Result<Self, ParseError> {
        let name = name.into();
        let mut lexer = Lexer::new_bytes(source, name.clone());
        let mut tokens = Vec::new();
        loop {
            match lexer.next_token() {
                Ok(token) => {
                    let at_end = token.kind == TokenKind::End;
                    tokens.push(token);
                    if at_end {
                        break;
                    }
                }
                Err(error) => {
                    let location = error.location;
                    tokens.push(Token::invalid(error));
                    tokens.push(Token {
                        kind: TokenKind::End,
                        lexeme: String::new(),
                        location,
                        end_column: location.column,
                        bytes: Vec::new(),
                    });
                    break;
                }
            }
        }
        let prefix_operators = ["not", "!", "-", "~"]
            .into_iter()
            .map(str::to_owned)
            .collect();
        let infix_operators = [
            ("or", 100_u64),
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
        .map(|(operator, precedence)| {
            (
                operator.to_owned(),
                (u128::from(precedence) << 64) | u128::from(u64::MAX),
            )
        })
        .collect();
        Ok(Self {
            tokens,
            current: 0,
            prefix_operators,
            infix_operators,
            custom_infix_count: 0,
            frozen_lookahead_operator: None,
            source_bytes: source.to_vec(),
            name,
        })
    }

    pub(crate) fn plain_error(location: Location, message: impl Into<String>) -> ParseError {
        ParseError {
            location,
            message: message.into(),
            context: None,
        }
    }

    pub fn parse(mut self) -> Result<Block, ParseError> {
        let mut statements = Vec::new();
        self.skip_semicolons();
        while !self.at(TokenKind::End) {
            let statement = self.statement()?;
            self.skip_semicolons();
            self.freeze_lookahead_operator();
            match &statement {
                Stmt::PrefixOperator(operator) => {
                    self.add_prefix_operator(operator.clone());
                }
                Stmt::InfixOperator {
                    operator,
                    precedence,
                } => {
                    self.add_infix_operator(operator.clone(), *precedence);
                }
                _ => {}
            }
            statements.push(statement);
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
        self.freeze_lookahead_operator();
        Ok(Some(statement))
    }

    pub(crate) fn is_at_end(&self) -> bool {
        self.at(TokenKind::End)
    }

    pub(crate) fn has_lex_error(&self) -> bool {
        self.tokens
            .iter()
            .any(|token| token.kind == TokenKind::Invalid)
    }

    pub fn add_prefix_operator(&mut self, operator: impl Into<String>) {
        self.prefix_operators.insert(operator.into());
    }

    pub fn add_infix_operator(&mut self, operator: impl Into<String>, precedence: u64) {
        let operator = operator.into();
        // Every custom operator gets a separate precedence layer immediately
        // before existing layers with the same numeric value. Earlier custom
        // declarations therefore bind more tightly, while built-ins at that
        // number bind more tightly than custom operators.
        let tie_break = u64::MAX
            .saturating_sub(1)
            .saturating_sub(self.custom_infix_count);
        self.custom_infix_count = self.custom_infix_count.saturating_add(1);
        let encoded = (u128::from(precedence) << 64) | u128::from(tie_break);
        self.infix_operators
            .entry(operator)
            .and_modify(|existing| *existing = (*existing).max(encoded))
            .or_insert(encoded);
    }

    fn statement(&mut self) -> Result<Stmt, ParseError> {
        match self.peek().kind {
            TokenKind::At => {
                self.advance();
                if matches!(
                    self.peek().kind,
                    TokenKind::LeftParen | TokenKind::Colon | TokenKind::LeftBrace
                ) {
                    // The statement dispatcher consumes the leading `@` while
                    // deciding between an expression and declaration, then
                    // parses the remainder as an ordinary expression.
                    Ok(Stmt::Expression(self.expression()?))
                } else {
                    Ok(Stmt::Declaration(self.declaration()?))
                }
            }
            TokenKind::Class => self.class_declaration(),
            TokenKind::Use => self.import(),
            TokenKind::PrefixOp => self.prefix_operator(),
            TokenKind::InfixOp => self.infix_operator(),
            TokenKind::If => self.if_statement(),
            TokenKind::Elif => Err(self.error("wrong token here")),
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
            TokenKind::Identifier if !self.current_is_dynamic_operator() => {
                Ok(Stmt::Expression(self.expression()?))
            }
            TokenKind::Identifier if self.current_is_prefix_operator() => {
                Ok(Stmt::Expression(self.expression()?))
            }
            TokenKind::Subtract
            | TokenKind::Integer
            | TokenKind::Float
            | TokenKind::None
            | TokenKind::True
            | TokenKind::False
            | TokenKind::String
            | TokenKind::LeftParen
            | TokenKind::Enum
            | TokenKind::Dict
            | TokenKind::Match
            | TokenKind::LeftBracket
            | TokenKind::LeftBrace
            | TokenKind::Not
            | TokenKind::BitNot
            | TokenKind::Invalid => Ok(Stmt::Expression(self.expression()?)),
            _ if self.current_is_prefix_operator() => Ok(Stmt::Expression(self.expression()?)),
            _ => Err(self.error("wrong token here")),
        }
    }

    fn declaration(&mut self) -> Result<Declaration, ParseError> {
        let first = self.binding(true)?;
        let mut bindings = vec![first];
        while self.take(TokenKind::Comma).is_some() {
            bindings.push(self.binding(false)?);
        }

        if matches!(
            bindings.as_slice(),
            [Binding::Name {
                by_reference: true,
                ..
            }]
        ) && self.at(TokenKind::LeftParen)
        {
            return Err(self.error("& cannot be here"));
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
            let body = self.function_body(true)?;
            return Ok(Declaration {
                bindings: vec![Binding::Name {
                    name,
                    by_reference: true,
                }],
                values: vec![Expr::Lambda { parameters, body }],
            });
        }

        let values = if self.take(TokenKind::Colon).is_some() {
            if matches!(bindings.as_slice(), [Binding::Name { .. }]) {
                vec![self.delay_expression()?]
            } else {
                self.expression_list()?
            }
        } else {
            Vec::new()
        };
        if values.is_empty() {
            if bindings.len() > 1
                && bindings
                    .iter()
                    .any(|binding| matches!(binding, Binding::Name { .. }))
            {
                return Err(Parser::plain_error(
                    self.peek().location,
                    "expect expressions",
                ));
            }
            if matches!(
                bindings.as_slice(),
                [Binding::Name {
                    by_reference: true,
                    ..
                }]
            ) {
                return Err(self.error("reference should be binded with other variable"));
            }
        }
        Ok(Declaration { bindings, values })
    }

    fn binding(&mut self, first: bool) -> Result<Binding, ParseError> {
        if self.take(TokenKind::LeftBracket).is_some() {
            let mut bindings = vec![self.binding(false)?];
            while self.take(TokenKind::Comma).is_some() {
                bindings.push(self.binding(false)?);
            }
            self.expect(TokenKind::RightBracket, "expect ']' here")?;
            return Ok(Binding::Destructure(bindings));
        }
        let by_reference = self.take(TokenKind::BitAnd).is_some();
        let name = if self.at(TokenKind::Identifier) && !self.current_is_dynamic_operator() {
            self.advance().lexeme.clone()
        } else if first {
            return Err(self.error("expect an identifier here"));
        } else {
            return Err(Parser::plain_error(
                self.peek().location,
                "expect an identifier",
            ));
        };
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
            if aliases
                .iter()
                .any(|alias| matches!(alias.module.last(), Some(ModulePart::Path(_))))
            {
                return Err(self.error(
                    "unexpected from because there is at least one module denoted by its path",
                ));
            }
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
        let is_dynamic_operator = self.current_is_dynamic_operator();
        let token = self.advance().clone();
        match token.kind {
            TokenKind::Identifier if !is_dynamic_operator => Ok(ModulePart::Name(token.lexeme)),
            TokenKind::String => Ok(ModulePart::Path(token.bytes)),
            _ => Err(ParseError {
                location: token.location,
                message: "expect a module here".to_owned(),
                context: Some(Box::new(self.error_context(token.location))),
            }),
        }
    }

    fn prefix_operator(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let operator = self.identifier("expected an identifier here")?;
        Ok(Stmt::PrefixOperator(operator))
    }

    fn infix_operator(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let precedence = if self.at(TokenKind::Integer) {
            let token = self.advance().clone();
            token
                .lexeme
                .parse()
                .map_err(|_| Parser::plain_error(token.location, "stoull"))?
        } else {
            50
        };
        let operator = self.identifier("expected an identifier here")?;
        Ok(Stmt::InfixOperator {
            operator,
            precedence,
        })
    }

    fn if_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let location = self.peek().location;
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
            location,
        })
    }

    fn while_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let location = self.peek().location;
        let condition = self.expression()?;
        let body = self.block()?;
        Ok(Stmt::While {
            condition,
            body,
            location,
        })
    }

    fn do_while_statement(&mut self) -> Result<Stmt, ParseError> {
        self.advance();
        let body = self.block()?;
        self.expect(TokenKind::While, "expected keyword 'while' after 'do'")?;
        let location = self.peek().location;
        let condition = self.expression()?;
        Ok(Stmt::DoWhile {
            body,
            condition,
            location,
        })
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
        if self.at(TokenKind::Semicolon) {
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

    fn binary_expression(&mut self, minimum_precedence: u128) -> Result<Expr, ParseError> {
        let mut left = self.prefix_expression()?;
        while let Some(precedence) = self.current_infix_precedence() {
            if precedence < minimum_precedence {
                break;
            }
            let operator = self.advance().clone();
            let right = self.binary_expression(precedence + 1)?;
            left = binary_expression(left, operator.lexeme, right, operator.location);
        }
        Ok(left)
    }

    fn prefix_expression(&mut self) -> Result<Expr, ParseError> {
        if self.current_is_prefix_operator() {
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
        let is_dynamic_operator = self.current_is_dynamic_operator();
        let token = self.advance().clone();
        match token.kind {
            TokenKind::None => Ok(Expr::Literal(Literal::None)),
            TokenKind::True => Ok(Expr::Literal(Literal::Bool(true))),
            TokenKind::False => Ok(Expr::Literal(Literal::Bool(false))),
            TokenKind::Integer => token
                .lexeme
                .parse()
                .map(|value| Expr::Literal(Literal::Integer(value)))
                .map_err(|_| Parser::plain_error(token.location, "stoll")),
            TokenKind::Float => token
                .lexeme
                .parse::<f64>()
                .ok()
                .filter(|value| value.is_finite())
                .map(|value| Expr::Literal(Literal::Float(value)))
                .ok_or_else(|| Parser::plain_error(token.location, "stod")),
            TokenKind::String => Ok(Expr::Literal(Literal::String(token.bytes))),
            TokenKind::Identifier if !is_dynamic_operator => {
                Ok(Expr::Identifier(token.lexeme, token.location))
            }
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
                    // Shorthand blocks use a synthetic call location.
                    location: Location { line: 0, column: 0 },
                })
            }
            TokenKind::Dict => self.dict(),
            TokenKind::Enum => self.enum_expression(),
            TokenKind::Match => self.match_expression(),
            TokenKind::Class => {
                self.current -= 1;
                self.class_expression()
            }
            TokenKind::Invalid => Err(Parser::plain_error(token.location, token.lexeme)),
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
        if let Some(colon) = self.take(TokenKind::Colon) {
            expression = binary_expression(
                expression,
                ":".to_owned(),
                self.delay_expression()?,
                colon.location,
            );
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
            body: self.function_body(false)?,
        })
    }

    fn parameters_after_left_paren(&mut self) -> Result<Vec<Parameter>, ParseError> {
        let mut parameters = Vec::new();
        let mut need_default = false;
        while !self.at(TokenKind::RightParen) {
            let variadic = self.take(TokenKind::Ellipsis).is_some();
            let by_reference = self.take(TokenKind::BitAnd).is_some();
            let name = self.identifier("expect an identifier here")?;
            if variadic && self.at(TokenKind::Colon) {
                return Err(self.error("expected ')' here"));
            }
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

    fn function_body(&mut self, allow_multiple_returns: bool) -> Result<Block, ParseError> {
        if self.take(TokenKind::Colon).is_some() {
            let expressions = if allow_multiple_returns {
                self.expression_list()?
            } else {
                vec![self.delay_expression()?]
            };
            Ok(vec![Stmt::Return(expressions)])
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
            self.take(TokenKind::Comma);
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
            let name = self.identifier("expect an identifier here")?;
            if self.take(TokenKind::Colon).is_some() {
                let token = self.expect(TokenKind::Integer, "expected integer here")?;
                next_value = token
                    .lexeme
                    .parse()
                    .map_err(|_| Parser::plain_error(token.location, "stoll"))?;
            }
            values.push((name, next_value));
            next_value = next_value
                .checked_add(1)
                .ok_or_else(|| self.error("enum value overflow"))?;
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
        let mut key_locations = Vec::new();
        let mut fallback = None;
        while !self.at(TokenKind::RightBrace) {
            if self.take(TokenKind::FatArrow).is_some() {
                if fallback.is_some() {
                    return Err(Parser::plain_error(
                        self.peek().location,
                        "redefinition of else-expr of match-expr",
                    ));
                }
                fallback = Some(Box::new(self.expression()?));
            } else {
                let mut locations = vec![self.peek().location];
                let mut keys = vec![self.expression()?];
                while self.take(TokenKind::Comma).is_some() {
                    if self.at(TokenKind::FatArrow) {
                        return Err(self.error("expected an expr here"));
                    }
                    locations.push(self.peek().location);
                    keys.push(self.expression()?);
                }
                self.expect(TokenKind::FatArrow, "expected symbol '=>'")?;
                arms.push((keys, self.expression()?));
                key_locations.push(locations);
            }
            if self.take(TokenKind::Comma).is_none() && !self.at(TokenKind::RightBrace) {
                return Err(self.error("expected '}'"));
            }
        }
        self.advance();
        Ok(Expr::Match {
            value: Box::new(value),
            arms,
            key_locations,
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
        let name = if self.at(TokenKind::Identifier) && !self.current_is_dynamic_operator() {
            Some(self.identifier("expected class name")?)
        } else {
            None
        };
        self.expect(TokenKind::LeftBrace, "expected '{'")?;
        let mut members = Vec::new();
        self.skip_semicolons();
        while !self.at(TokenKind::RightBrace) {
            if self.at(TokenKind::At) {
                return Err(self.error("expect an identifier here"));
            }
            let mut member = self.declaration()?;
            let [Binding::Name { name, .. }] = member.bindings.as_slice() else {
                return Err(Parser::plain_error(
                    self.peek().location,
                    "not support multi-declaration in class",
                ));
            };
            if name == "__init__" {
                let [Expr::Lambda { parameters, body }] = member.values.as_mut_slice() else {
                    return Err(self.error("__init__ must be with function body"));
                };
                let Some(receiver) = parameters.first() else {
                    return Err(Parser::plain_error(
                        self.peek().location,
                        "method need at least 1 parameter",
                    ));
                };
                body.push(Stmt::Return(vec![Expr::Identifier(
                    receiver.name.clone(),
                    Location { line: 0, column: 0 },
                )]));
            }
            members.push(member);
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
        if self.at(TokenKind::Identifier) && !self.current_is_dynamic_operator() {
            Ok(self.advance().lexeme.clone())
        } else {
            Err(self.error(message))
        }
    }

    fn freeze_lookahead_operator(&mut self) {
        self.frozen_lookahead_operator = (self.peek().kind == TokenKind::Identifier).then(|| {
            (
                self.current,
                self.prefix_operators.contains(&self.peek().lexeme),
                self.infix_operators.get(&self.peek().lexeme).copied(),
            )
        });
    }

    fn current_is_prefix_operator(&self) -> bool {
        self.frozen_lookahead_operator
            .filter(|(index, _, _)| *index == self.current)
            .map_or_else(
                || self.prefix_operators.contains(&self.peek().lexeme),
                |(_, is_prefix, _)| is_prefix,
            )
    }

    fn current_infix_precedence(&self) -> Option<u128> {
        self.frozen_lookahead_operator
            .filter(|(index, _, _)| *index == self.current)
            .map_or_else(
                || self.infix_operators.get(&self.peek().lexeme).copied(),
                |(_, _, precedence)| precedence,
            )
    }

    fn current_is_dynamic_operator(&self) -> bool {
        self.current_is_prefix_operator() || self.current_infix_precedence().is_some()
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
        if self.peek().kind == TokenKind::Invalid {
            return Parser::plain_error(self.peek().location, self.peek().lexeme.clone());
        }
        self.error_at(self.peek().location, message)
    }

    fn error_at(&self, location: Location, message: impl Into<String>) -> ParseError {
        ParseError {
            location,
            message: message.into(),
            context: Some(Box::new(self.error_context(location))),
        }
    }

    fn error_context(&self, location: Location) -> ParseErrorContext {
        let full_line = self
            .source_bytes
            .split(|byte| *byte == b'\n')
            .nth(location.line.saturating_sub(1))
            .unwrap_or_default();
        let end_column = self
            .tokens
            .iter()
            .find(|token| token.location == location)
            .map_or(location.column, |token| token.end_column);
        let lookahead = full_line.get(end_column..).map_or(0, |_| 1);
        let visible_end = end_column.saturating_add(lookahead).min(full_line.len());
        let line_bytes = full_line[..visible_end].to_vec();
        ParseErrorContext {
            name: self.name.clone(),
            line: String::from_utf8_lossy(&line_bytes).into_owned(),
            name_bytes: self.name.as_bytes().to_vec(),
            line_bytes,
        }
    }
}

fn binary_expression(left: Expr, operator: String, right: Expr, location: Location) -> Expr {
    if let (Expr::Literal(Literal::Integer(left)), Expr::Literal(Literal::Integer(right))) =
        (&left, &right)
    {
        let folded = match operator.as_str() {
            "+" => left.checked_add(*right).map(Literal::Integer),
            "-" => left.checked_sub(*right).map(Literal::Integer),
            "*" => left.checked_mul(*right).map(Literal::Integer),
            "/" if *right != 0 => left.checked_div(*right).map(Literal::Integer),
            "%" if *right != 0 => left.checked_rem(*right).map(Literal::Integer),
            "and" => Some(Literal::Bool(*left != 0 && *right != 0)),
            "or" => Some(Literal::Bool(*left != 0 || *right != 0)),
            "is" | "=" => Some(Literal::Bool(left == right)),
            "!=" => Some(Literal::Bool(left != right)),
            "<" => Some(Literal::Bool(left < right)),
            "<=" => Some(Literal::Bool(left <= right)),
            ">" => Some(Literal::Bool(left > right)),
            ">=" => Some(Literal::Bool(left >= right)),
            _ => None,
        };
        if let Some(folded) = folded {
            return Expr::Literal(folded);
        }
    }
    Expr::Binary {
        left: Box::new(left),
        operator,
        right: Box::new(right),
        location,
    }
}
