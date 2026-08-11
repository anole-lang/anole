use anole::{Lexer, Location, TokenKind};

#[test]
fn tokenizes_the_legacy_origin_sequence() {
    let input = r#"@
use from prefixop infixop if elif else
while do foreach as break continue return
match delay enum dict class none true false
identifier 0123456789 0123456789.0123456789 "String"
, . ... () [] {} : ; + - * / % & | ^ ~ << >>
and or not ! is = != < <= > >= => ?"#;
    let actual: Vec<_> = Lexer::new(input, "<test>")
        .tokenize()
        .unwrap()
        .into_iter()
        .map(|token| token.kind)
        .collect();
    let expected = vec![
        TokenKind::At,
        TokenKind::Use,
        TokenKind::From,
        TokenKind::PrefixOp,
        TokenKind::InfixOp,
        TokenKind::If,
        TokenKind::Elif,
        TokenKind::Else,
        TokenKind::While,
        TokenKind::Do,
        TokenKind::Foreach,
        TokenKind::As,
        TokenKind::Break,
        TokenKind::Continue,
        TokenKind::Return,
        TokenKind::Match,
        TokenKind::Delay,
        TokenKind::Enum,
        TokenKind::Dict,
        TokenKind::Class,
        TokenKind::None,
        TokenKind::True,
        TokenKind::False,
        TokenKind::Identifier,
        TokenKind::Integer,
        TokenKind::Float,
        TokenKind::String,
        TokenKind::Comma,
        TokenKind::Dot,
        TokenKind::Ellipsis,
        TokenKind::LeftParen,
        TokenKind::RightParen,
        TokenKind::LeftBracket,
        TokenKind::RightBracket,
        TokenKind::LeftBrace,
        TokenKind::RightBrace,
        TokenKind::Colon,
        TokenKind::Semicolon,
        TokenKind::Add,
        TokenKind::Subtract,
        TokenKind::Multiply,
        TokenKind::Divide,
        TokenKind::Remainder,
        TokenKind::BitAnd,
        TokenKind::BitOr,
        TokenKind::BitXor,
        TokenKind::BitNot,
        TokenKind::ShiftLeft,
        TokenKind::ShiftRight,
        TokenKind::And,
        TokenKind::Or,
        TokenKind::Not,
        TokenKind::Not,
        TokenKind::Is,
        TokenKind::Equal,
        TokenKind::NotEqual,
        TokenKind::Less,
        TokenKind::LessEqual,
        TokenKind::Greater,
        TokenKind::GreaterEqual,
        TokenKind::FatArrow,
        TokenKind::Question,
        TokenKind::End,
    ];
    assert_eq!(actual, expected);
}

#[test]
fn preserves_legacy_string_escaping_and_unknown_escapes() {
    let token = Lexer::new(r#""a\n\t\"\\\q""#, "<test>")
        .next_token()
        .unwrap();
    assert_eq!(token.kind, TokenKind::String);
    assert_eq!(token.lexeme, "a\n\t\"\\\\q");
}

#[test]
fn skips_line_comments_but_keeps_division_tokens() {
    let tokens = Lexer::new("1 / 2 // ignore me\n3", "<test>")
        .tokenize()
        .unwrap();
    let kinds: Vec<_> = tokens.into_iter().map(|token| token.kind).collect();
    assert_eq!(
        kinds,
        [
            TokenKind::Integer,
            TokenKind::Divide,
            TokenKind::Integer,
            TokenKind::Integer,
            TokenKind::End
        ]
    );
}

#[test]
fn reports_the_start_of_an_unterminated_string() {
    let error = Lexer::new("\n  \"unterminated", "sample.anole")
        .next_token()
        .unwrap_err();
    assert_eq!(error.location, Location { line: 2, column: 2 });
    assert!(error.to_string().contains("sample.anole:2:2: error"));
}

#[test]
fn rejects_two_dots_like_the_legacy_tokenizer() {
    let error = Lexer::new("..", "<test>").next_token().unwrap_err();
    assert_eq!(error.message, "unexpected \"..\"");
}
