use anole::{Lexer, TokenKind};

// This test intentionally constructs source that cannot be represented by a
// UTF-8 `.anole` fixture. All ordinary lexer behavior lives in compile tests.
#[test]
fn treats_source_byte_ff_as_the_signed_char_eof_sentinel() {
    let tokens = Lexer::new_bytes(b"left;\xff right;", "bytes.anole")
        .tokenize()
        .unwrap();
    assert_eq!(
        tokens.iter().map(|token| &token.kind).collect::<Vec<_>>(),
        vec![
            &TokenKind::Identifier,
            &TokenKind::Semicolon,
            &TokenKind::End
        ]
    );

    let tokens = Lexer::new_bytes(b"// ignored\xffright;", "bytes.anole")
        .tokenize()
        .unwrap();
    assert_eq!(
        tokens.iter().map(|token| &token.kind).collect::<Vec<_>>(),
        vec![
            &TokenKind::Identifier,
            &TokenKind::Semicolon,
            &TokenKind::End
        ]
    );
    assert_eq!(tokens[0].lexeme, "right");
}
