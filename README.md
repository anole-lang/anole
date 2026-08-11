# Anole Programming Language

Anole is a small dynamically typed language. The interpreter is implemented in
safe, idiomatic Rust and preserves the observable behavior of the former C++
0.0.24 implementation, including dynamic operators, lazy values, references,
classes, modules and first-class continuations.

## Toolchain

The repository pins the latest stable Rust release used by this rewrite in
`rust-toolchain.toml`. Rustup selects it automatically.

## Build and install

```bash
cargo build --release
cargo install --path .
```

The standard `env`, `file`, `os`, `debug`, and `coroutine` modules are embedded
in the binary. An installed interpreter therefore has no C++ shared-library or
external runtime dependency.

## Usage

Run a source file:

```bash
anole example/class.anole
```

Pass arguments to a program:

```bash
anole example/env_args.anole first second
```

Start the REPL with `anole`, or pipe a program through standard input. Print the
compatible version literal with `anole --version`.

## Development

The rewrite follows a test-first compatibility suite derived from the original
tokenizer tests, runtime samples, command-line behavior and repository examples.

```bash
cargo fmt --all -- --check
cargo test --all-targets
cargo clippy --all-targets -- -D warnings
```

The implementation is split into a lexer, Rust enum-based AST, Pratt parser and
a trampolined continuation-passing runtime. The trampoline makes loops, deep
functional composition and resumable continuations independent of the native
call-stack depth.

See [ChangeLog.md](ChangeLog.md) for the historical language changes.
