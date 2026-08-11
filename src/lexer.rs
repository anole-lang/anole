use std::fmt;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct Location {
    pub line: usize,
    pub column: usize,
}

impl Location {
    const START: Self = Self { line: 1, column: 0 };
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum TokenKind {
    At,
    Use,
    From,
    PrefixOp,
    InfixOp,
    If,
    Elif,
    Else,
    While,
    Do,
    Foreach,
    As,
    Break,
    Continue,
    Return,
    Match,
    Delay,
    Enum,
    Dict,
    Class,
    None,
    True,
    False,
    Identifier,
    Integer,
    Float,
    String,
    Comma,
    Dot,
    Ellipsis,
    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Colon,
    Semicolon,
    Add,
    Subtract,
    Multiply,
    Divide,
    Remainder,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    ShiftLeft,
    ShiftRight,
    And,
    Or,
    Not,
    Is,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    FatArrow,
    Question,
    End,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Token {
    pub kind: TokenKind,
    pub lexeme: String,
    pub location: Location,
}

impl Token {
    fn new(kind: TokenKind, lexeme: impl Into<String>, location: Location) -> Self {
        Self {
            kind,
            lexeme: lexeme.into(),
            location,
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct LexError {
    pub name: String,
    pub location: Location,
    pub message: String,
    pub line: String,
}

impl fmt::Display for LexError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        writeln!(
            f,
            "{}:{}:{}: error: {}",
            self.name, self.location.line, self.location.column, self.message
        )?;
        writeln!(f, "{}", self.line)?;
        write!(f, "{}^", " ".repeat(self.location.column.saturating_sub(1)))
    }
}

impl std::error::Error for LexError {}

pub struct Lexer<'source> {
    source: &'source str,
    name: String,
    offset: usize,
    location: Location,
}

impl<'source> Lexer<'source> {
    #[must_use]
    pub fn new(source: &'source str, name: impl Into<String>) -> Self {
        Self {
            source,
            name: name.into(),
            offset: 0,
            location: Location::START,
        }
    }

    pub fn tokenize(mut self) -> Result<Vec<Token>, LexError> {
        let mut tokens = Vec::new();
        loop {
            let token = self.next_token()?;
            let at_end = token.kind == TokenKind::End;
            tokens.push(token);
            if at_end {
                return Ok(tokens);
            }
        }
    }

    pub fn next_token(&mut self) -> Result<Token, LexError> {
        self.skip_trivia();
        let location = self.location;
        let Some(current) = self.peek() else {
            return Ok(Token::new(TokenKind::End, "", location));
        };

        if current.is_ascii_digit() {
            return Ok(self.number(location));
        }
        if is_normal_identifier_start(current) {
            return Ok(self.normal_identifier(location));
        }

        match current {
            '@' => Ok(self.single(TokenKind::At, location)),
            '&' => Ok(self.single(TokenKind::BitAnd, location)),
            ':' => Ok(self.single(TokenKind::Colon, location)),
            ';' => Ok(self.single(TokenKind::Semicolon, location)),
            ',' => Ok(self.single(TokenKind::Comma, location)),
            '(' => Ok(self.single(TokenKind::LeftParen, location)),
            ')' => Ok(self.single(TokenKind::RightParen, location)),
            '[' => Ok(self.single(TokenKind::LeftBracket, location)),
            ']' => Ok(self.single(TokenKind::RightBracket, location)),
            '{' => Ok(self.single(TokenKind::LeftBrace, location)),
            '}' => Ok(self.single(TokenKind::RightBrace, location)),
            '?' => Ok(self.single(TokenKind::Question, location)),
            '.' => self.dot(location),
            '"' => self.string(location),
            _ if is_abnormal_identifier_char(current) => Ok(self.abnormal_identifier(location)),
            _ => Err(self.error(location, format!("unexpected character {current:?}"))),
        }
    }

    fn skip_trivia(&mut self) {
        loop {
            while self
                .peek()
                .is_some_and(|character| character.is_ascii_whitespace())
            {
                self.advance();
            }
            if self.remaining().starts_with("//") {
                while self.peek().is_some_and(|character| character != '\n') {
                    self.advance();
                }
                continue;
            }
            break;
        }
    }

    fn number(&mut self, location: Location) -> Token {
        let start = self.offset;
        while self
            .peek()
            .is_some_and(|character| character.is_ascii_digit())
        {
            self.advance();
        }
        let kind = if self.peek() == Some('.') {
            self.advance();
            while self
                .peek()
                .is_some_and(|character| character.is_ascii_digit())
            {
                self.advance();
            }
            TokenKind::Float
        } else {
            TokenKind::Integer
        };
        Token::new(kind, &self.source[start..self.offset], location)
    }

    fn normal_identifier(&mut self, location: Location) -> Token {
        let start = self.offset;
        while self.peek().is_some_and(is_normal_identifier_char) {
            self.advance();
        }
        let value = &self.source[start..self.offset];
        Token::new(keyword_or_identifier(value), value, location)
    }

    fn abnormal_identifier(&mut self, location: Location) -> Token {
        let start = self.offset;
        while self.peek().is_some_and(is_abnormal_identifier_char) {
            self.advance();
        }
        let value = &self.source[start..self.offset];
        Token::new(keyword_or_identifier(value), value, location)
    }

    fn dot(&mut self, location: Location) -> Result<Token, LexError> {
        self.advance();
        if self.peek() != Some('.') {
            return Ok(Token::new(TokenKind::Dot, ".", location));
        }
        self.advance();
        if self.peek() != Some('.') {
            return Err(self.error(location, "unexpected \"..\""));
        }
        self.advance();
        Ok(Token::new(TokenKind::Ellipsis, "...", location))
    }

    fn string(&mut self, location: Location) -> Result<Token, LexError> {
        self.advance();
        let mut value = String::new();
        loop {
            let Some(character) = self.advance() else {
                return Err(self.error(location, "expected \\\""));
            };
            match character {
                '"' => return Ok(Token::new(TokenKind::String, value, location)),
                '\n' => return Err(self.error(location, "expected \\\"")),
                '\\' => {
                    let Some(escaped) = self.advance() else {
                        return Err(self.error(location, "expected \\\""));
                    };
                    match escaped {
                        '\n' => return Err(self.error(location, "expected \\\"")),
                        'n' => value.push('\n'),
                        '\\' => value.push('\\'),
                        '"' => value.push('"'),
                        'a' => value.push('\x07'),
                        'b' => value.push('\x08'),
                        '0' => value.push('\0'),
                        't' => value.push('\t'),
                        'r' => value.push('\r'),
                        'f' => value.push('\x0c'),
                        unknown => {
                            value.push('\\');
                            value.push(unknown);
                        }
                    }
                }
                other => value.push(other),
            }
        }
    }

    fn single(&mut self, kind: TokenKind, location: Location) -> Token {
        let character = self
            .advance()
            .expect("single token starts with a character");
        Token::new(kind, character, location)
    }

    fn peek(&self) -> Option<char> {
        self.remaining().chars().next()
    }

    fn remaining(&self) -> &'source str {
        &self.source[self.offset..]
    }

    fn advance(&mut self) -> Option<char> {
        let character = self.peek()?;
        self.offset += character.len_utf8();
        if character == '\n' {
            self.location.line += 1;
            self.location.column = 0;
        } else {
            self.location.column += 1;
        }
        Some(character)
    }

    fn error(&self, location: Location, message: impl Into<String>) -> LexError {
        let line = self
            .source
            .lines()
            .nth(location.line.saturating_sub(1))
            .unwrap_or_default()
            .to_owned();
        LexError {
            name: self.name.clone(),
            location,
            message: message.into(),
            line,
        }
    }
}

fn is_normal_identifier_start(character: char) -> bool {
    character.is_ascii_alphabetic() || character == '_'
}

fn is_normal_identifier_char(character: char) -> bool {
    character.is_ascii_alphanumeric() || character == '_'
}

fn is_abnormal_identifier_char(character: char) -> bool {
    !character.is_ascii_whitespace()
        && !is_normal_identifier_char(character)
        && !matches!(
            character,
            '@' | '.' | ',' | ':' | ';' | '"' | '?' | '&' | '(' | ')' | '[' | ']' | '{' | '}'
        )
}

fn keyword_or_identifier(value: &str) -> TokenKind {
    match value {
        "use" => TokenKind::Use,
        "from" => TokenKind::From,
        "prefixop" => TokenKind::PrefixOp,
        "infixop" => TokenKind::InfixOp,
        "if" => TokenKind::If,
        "elif" => TokenKind::Elif,
        "else" => TokenKind::Else,
        "while" => TokenKind::While,
        "do" => TokenKind::Do,
        "foreach" => TokenKind::Foreach,
        "as" => TokenKind::As,
        "break" => TokenKind::Break,
        "continue" => TokenKind::Continue,
        "return" => TokenKind::Return,
        "match" => TokenKind::Match,
        "delay" => TokenKind::Delay,
        "enum" => TokenKind::Enum,
        "dict" => TokenKind::Dict,
        "class" => TokenKind::Class,
        "none" => TokenKind::None,
        "true" => TokenKind::True,
        "false" => TokenKind::False,
        "+" => TokenKind::Add,
        "-" => TokenKind::Subtract,
        "*" => TokenKind::Multiply,
        "/" => TokenKind::Divide,
        "%" => TokenKind::Remainder,
        "|" => TokenKind::BitOr,
        "^" => TokenKind::BitXor,
        "~" => TokenKind::BitNot,
        "<<" => TokenKind::ShiftLeft,
        ">>" => TokenKind::ShiftRight,
        "and" => TokenKind::And,
        "or" => TokenKind::Or,
        "not" | "!" => TokenKind::Not,
        "is" => TokenKind::Is,
        "=" => TokenKind::Equal,
        "!=" => TokenKind::NotEqual,
        "<" => TokenKind::Less,
        "<=" => TokenKind::LessEqual,
        ">" => TokenKind::Greater,
        ">=" => TokenKind::GreaterEqual,
        "=>" => TokenKind::FatArrow,
        _ => TokenKind::Identifier,
    }
}
