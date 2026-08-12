# Compile tests

Compile tests are discovered recursively. A test consists of an Anole source
file and a same-named `.result` file. Optional `.stdout` and `.stderr` files
contain exact golden output. An `.anole` file without a `.result` file is a
fixture or module dependency, not a separate test.

The result file selects one of four modes:

```text
mode: run
status: success
```

- `run` executes the source with the Rust interpreter.
- `parse` writes the parsed AST to `.stdout`.
- `lex` writes one token per line to `.stdout`.
- `cli` runs the `anole` executable and records its exit status, stdout, and
  stderr. Repeat `option:` for interpreter options and `arg:` for program
  arguments.

Use `no-final-newline: true` when the physical fixture newline must not be part
of the source. Temporary directory paths in output are normalized to
`$TEST_DIR`, keeping filesystem diagnostics stable.

Failures use a structured result so error behavior stays readable:

```text
mode: run
status: failure
message: integer division by zero
line: 1
column: 25
```

Put `main.anole` and any dependency files in their own directory for module
tests. The complete directory is copied to a temporary location before each
test, so generated IR and files never affect the source tree.

Run all cases with `cargo test --test compile_tests`. Use a test argument to
select cases by path:

```sh
cargo test --test compile_tests -- --filter continuation
```

To create or update `.result`, `.stdout`, and `.stderr` files after an
intentional behavior change, pass `--bless`:

```sh
cargo test --test compile_tests -- --bless
```

The two arguments can be combined to update only selected cases.
