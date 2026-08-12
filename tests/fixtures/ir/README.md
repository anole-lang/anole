# IR fixtures

The `*.ir.hex` files are checked-in snapshots of Anole's serialized IR. Each
source was executed successfully and its generated `<source>.ir` was encoded
as hexadecimal text.

Together the fixtures exercise every serializable opcode from `Pop` (1) through
`BuildClass` (51), all operand encodings, constant de-duplication and folding,
source mappings, control-flow patching, constructor rewriting, imported
modules, and dynamically imported operators. `PlaceHolder` (0) is an internal
code-generation marker and is patched before valid code is written.

`tests/cli.rs` compares generated IR with these checked-in snapshots.
