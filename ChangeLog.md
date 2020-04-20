# ChangeLog for Anole

## 0.0.10 - 2020/04/20

### Added

- Instruction `StoreRef`
- Support pass-by-reference now with syntax `@(&parameter) { ... }`
- Support declare a variable as reference

### Changed

- Function will return the reference instead of rebinding the object to a temporary variable
- Keyboard interrupt will cancel the inputing of current statement instead of current line

### Fixed

- Fix bug when module's code has definitions of custom operators, actions are interpreting each statement immediately after it is parsed instead of interpreting statements after all are parsed

## 0.0.9 - 2020/04/17

### Added

- Insturction AddInfixOp to make identifier as infix operator
- Support custom operators and priorities
- Demo to define custom operators

### Changed

- The non-REPL will interpret statement while it is parsed instead of execute after parsing all statements

## 0.0.8 - 2020/04/14

### Changed

- Rename to Anole

## 0.0.7 - 2020/04/12

### Added

- Complete the codegen of `foreach`
- Add the example of using `foreach`

### Changed

- Use unique_ptr instead of shared_ptr in AST

### Removed

- Remove the old support for `for stmt`
- Remove old testers for tokenizer and parser

## 0.0.6 - 2020/04/10

### Added

- IR will be stored as *.icei

### Updated

- Code will unserialize from the IR file if the IR file is newer than the corresponding source file

### Fixed

- Fix bug that context cannot traceback when interpreting modules

## 0.0.5 - 2020/04/09

### Added

- Support traceback when meeting runtime error
- CompileError/RuntimeError to distinguish the two exceptions

### Updated

- Runtime error will show the running position

## 0.0.4 - 2020/04/08

### Added

- Methods tellg/tellp, seekg/seekp, good and eof for fileobject
- Support enum { \[ident \[: integer\]\]* }

### Changed

- Coroutine as a library now rather than the example
- Suffix from .anole to .icec

## 0.0.3 - 2020/04/07

### Added

- Support history at REPL mode
- Prompt undefined symbol with its name
- Directory `lib/` for standard libraries
- Add standard library `file`
- Instructions to support binary operations now

### Updated

- Parser could parse binary operations now

### Changed

- Console will wait for new input after inputing `Ctrl ^ C`

### Fixed

- Exit rather than infinite loop when receiving `Ctrl ^ D`
- Fix bug in modules finding

## 0.0.2 - 2020/04/04

### Added

- Keyword `from` to use in `use` stmt
- Instruction `Import`, `ImportPart` and `ImportAll` to support use statement

### Changed

- Update `use ident` to `use (ident [as ident]) [, ident [as ident]]* [from (ident)]` or `use * from (ident)`
- Use statement can import module from .so files
- Module can be defined as a directory

### Removed

- Instruction `Use`

## 0.0.1 - 2020/04/01

### Added

- This ChangeLog file to document changes of Anole

### Removed

- The origin ChangeLog in README.md
- The Samples.md, because I plan to write some docs later and you can learn something about how to use Anole by examples in [example/](example/)

## 0.0.0

### Others

- Groundbreak! Everything already exists!
