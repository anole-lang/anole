# Anole Programming Language

Anole is a small dynamically typed language implemented in safe, idiomatic
Rust. It supports dynamic operators, lazy values, references, classes, modules,
and first-class continuations.

## Toolchain

The repository tracks the Rust `stable` channel in `rust-toolchain.toml`.
Rustup selects the current stable toolchain automatically.

## Build and install

```bash
cargo build --release
cargo install --path .
```

The standard `env`, `file`, `os`, `debug`, and `coroutine` modules are embedded
in the binary, so an installed interpreter has no external runtime dependency.

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

Successful file execution writes `<source>.ir`, including for imported Anole
modules. The cache uses native-endian 64-bit fields, magic value `20210213`, a
constant pool and opcode stream, followed by the source-position map. No IR file
is written when parsing or execution fails.

## Development

The interpreter is covered by behavior tests for tokenization, parsing, IR,
runtime semantics, command-line behavior, standard modules, and examples.

```bash
cargo fmt --all -- --check
cargo test --all-targets
cargo clippy --all-targets -- -D warnings
```

The implementation is split into a lexer, Rust enum-based AST, Pratt parser,
bytecode generator and an explicit VM. `call_with_current_continuation` captures
a copy of the VM context, including its operand stack, scope, program counter,
and parent-context chain; execution does not depend on native call-stack depth.
