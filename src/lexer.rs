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
    #[doc(hidden)]
    Invalid,
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
    pub(crate) end_column: usize,
    pub(crate) bytes: Vec<u8>,
}

impl Token {
    fn new(
        kind: TokenKind,
        lexeme: impl Into<String>,
        location: Location,
        end_column: usize,
    ) -> Self {
        let lexeme = lexeme.into();
        Self {
            kind,
            bytes: lexeme.as_bytes().to_vec(),
            lexeme,
            location,
            end_column,
        }
    }

    fn new_bytes(kind: TokenKind, bytes: Vec<u8>, location: Location, end_column: usize) -> Self {
        Self {
            kind,
            lexeme: String::from_utf8_lossy(&bytes).into_owned(),
            bytes,
            location,
            end_column,
        }
    }

    pub(crate) fn invalid(error: LexError) -> Self {
        Self {
            kind: TokenKind::Invalid,
            bytes: error.message.as_bytes().to_vec(),
            lexeme: error.message,
            location: error.location,
            end_column: error.location.column,
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
    source: &'source [u8],
    name: String,
    offset: usize,
    location: Location,
}

impl<'source> Lexer<'source> {
    #[must_use]
    pub fn new(source: &'source str, name: impl Into<String>) -> Self {
        Self::new_bytes(source.as_bytes(), name)
    }

    #[must_use]
    pub fn new_bytes(source: &'source [u8], name: impl Into<String>) -> Self {
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
        let mut location = self.location;
        let Some(current) = self.peek() else {
            if location.column > 0 {
                location.column -= 1;
            }
            return Ok(Token::new(TokenKind::End, "", location, location.column));
        };

        if current.is_ascii_digit() {
            return Ok(self.number(location));
        }
        if is_normal_identifier_start(current) {
            return Ok(self.normal_identifier(location));
        }

        match current {
            b'@' => Ok(self.single(TokenKind::At, location)),
            b'&' => Ok(self.single(TokenKind::BitAnd, location)),
            b':' => Ok(self.single(TokenKind::Colon, location)),
            b';' => Ok(self.single(TokenKind::Semicolon, location)),
            b',' => Ok(self.single(TokenKind::Comma, location)),
            b'(' => Ok(self.single(TokenKind::LeftParen, location)),
            b')' => Ok(self.single(TokenKind::RightParen, location)),
            b'[' => Ok(self.single(TokenKind::LeftBracket, location)),
            b']' => Ok(self.single(TokenKind::RightBracket, location)),
            b'{' => Ok(self.single(TokenKind::LeftBrace, location)),
            b'}' => Ok(self.single(TokenKind::RightBrace, location)),
            b'?' => Ok(self.single(TokenKind::Question, location)),
            b'.' => self.dot(location),
            b'"' => self.string(location),
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
            if self.remaining().starts_with(b"//") {
                while self
                    .raw_peek()
                    .is_some_and(|character| !matches!(character, b'\n' | u8::MAX))
                {
                    self.advance();
                }
                if self.raw_peek() == Some(u8::MAX) {
                    // `istream::get()` was stored in a signed `char`; 0xff
                    // therefore compared equal to EOF. The line-comment
                    // state performed one more read before resuming.
                    self.offset += 1;
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
        let kind = if self.peek() == Some(b'.') {
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
        Token::new(
            kind,
            std::str::from_utf8(&self.source[start..self.offset])
                .expect("numeric tokens are ASCII"),
            location,
            self.location.column,
        )
    }

    fn normal_identifier(&mut self, location: Location) -> Token {
        let start = self.offset;
        while self.peek().is_some_and(is_normal_identifier_char) {
            self.advance();
        }
        let value = std::str::from_utf8(&self.source[start..self.offset])
            .expect("normal identifiers are ASCII");
        Token::new(
            keyword_or_identifier(value),
            value,
            location,
            self.location.column,
        )
    }

    fn abnormal_identifier(&mut self, location: Location) -> Token {
        let start = self.offset;
        while self.peek().is_some_and(is_abnormal_identifier_char) {
            self.advance();
        }
        if self.peek().is_none() {
            // EOF in this state discards the pending abnormal token.
            return Token::new(TokenKind::End, "", location, self.location.column);
        }
        let bytes = self.source[start..self.offset].to_vec();
        let value = symbol_from_bytes(&bytes);
        Token {
            kind: keyword_or_identifier(&value),
            lexeme: value,
            location,
            end_column: self.location.column,
            bytes,
        }
    }

    fn dot(&mut self, location: Location) -> Result<Token, LexError> {
        self.advance();
        if self.peek() != Some(b'.') {
            return Ok(Token::new(
                TokenKind::Dot,
                ".",
                location,
                self.location.column,
            ));
        }
        self.advance();
        if self.peek() != Some(b'.') {
            return Err(self.error(location, "unexpected \"..\""));
        }
        self.advance();
        Ok(Token::new(
            TokenKind::Ellipsis,
            "...",
            location,
            self.location.column,
        ))
    }

    fn string(&mut self, location: Location) -> Result<Token, LexError> {
        self.advance();
        let mut value = Vec::new();
        loop {
            let Some(character) = self.advance() else {
                // EOF terminates the token; only a newline diagnoses a missing
                // closing quote.
                return Ok(Token::new(
                    TokenKind::End,
                    "",
                    location,
                    self.location.column,
                ));
            };
            match character {
                b'"' => {
                    return Ok(Token::new_bytes(
                        TokenKind::String,
                        value,
                        location,
                        self.location.column,
                    ));
                }
                b'\n' => return Err(self.error(location, "expected \"")),
                b'\\' => {
                    let Some(escaped) = self.advance() else {
                        return Ok(Token::new(
                            TokenKind::End,
                            "",
                            location,
                            self.location.column,
                        ));
                    };
                    match escaped {
                        b'\n' => return Err(self.error(location, "expected \"")),
                        b'n' => value.push(b'\n'),
                        b'\\' => value.push(b'\\'),
                        b'"' => value.push(b'"'),
                        b'a' => value.push(0x07),
                        b'b' => value.push(0x08),
                        b'0' => value.push(0),
                        b't' => value.push(b'\t'),
                        b'r' => value.push(b'\r'),
                        b'f' => value.push(0x0c),
                        unknown => {
                            value.push(b'\\');
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
        Token::new(
            kind,
            char::from(character).to_string(),
            location,
            self.location.column,
        )
    }

    fn peek(&self) -> Option<u8> {
        self.raw_peek().filter(|character| *character != u8::MAX)
    }

    fn raw_peek(&self) -> Option<u8> {
        self.source.get(self.offset).copied()
    }

    fn remaining(&self) -> &'source [u8] {
        &self.source[self.offset..]
    }

    fn advance(&mut self) -> Option<u8> {
        let character = self.peek()?;
        self.offset += 1;
        if character == b'\n' {
            self.location.line += 1;
            self.location.column = 0;
        } else {
            // Columns count bytes because positions are serialized in `.ir`
            // source mappings.
            self.location.column += 1;
        }
        Some(character)
    }

    fn error(&self, location: Location, message: impl Into<String>) -> LexError {
        let line = self
            .source
            .split(|byte| *byte == b'\n')
            .nth(location.line.saturating_sub(1))
            .map_or_else(String::new, |line| {
                String::from_utf8_lossy(line).into_owned()
            });
        LexError {
            name: self.name.clone(),
            location,
            message: message.into(),
            line,
        }
    }
}

// Symbol names can contain arbitrary bytes. Keep ASCII unchanged and map every
// non-ASCII byte to a private-use scalar. Encoding valid UTF-8 byte-by-byte as
// well keeps the mapping injective even for private-use source text.
pub(crate) fn symbol_from_bytes(bytes: &[u8]) -> String {
    bytes
        .iter()
        .map(|byte| {
            if byte.is_ascii() {
                char::from(*byte)
            } else {
                char::from_u32(0xe000 + u32::from(*byte)).expect("private-use symbol byte")
            }
        })
        .collect()
}

pub(crate) fn symbol_to_bytes(symbol: &str) -> Vec<u8> {
    let mut bytes = Vec::with_capacity(symbol.len());
    for character in symbol.chars() {
        let value = u32::from(character);
        if (0xe080..=0xe0ff).contains(&value) {
            bytes.push((value - 0xe000) as u8);
        } else {
            let mut encoded = [0; 4];
            bytes.extend_from_slice(character.encode_utf8(&mut encoded).as_bytes());
        }
    }
    bytes
}

fn is_normal_identifier_start(character: u8) -> bool {
    character.is_ascii_alphabetic() || character == b'_'
}

fn is_normal_identifier_char(character: u8) -> bool {
    character.is_ascii_alphanumeric() || character == b'_'
}

fn is_abnormal_identifier_char(character: u8) -> bool {
    !character.is_ascii_whitespace()
        && !is_normal_identifier_char(character)
        && !matches!(
            character,
            b'@' | b'.'
                | b','
                | b':'
                | b';'
                | b'"'
                | b'?'
                | b'&'
                | b'('
                | b')'
                | b'['
                | b']'
                | b'{'
                | b'}'
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
