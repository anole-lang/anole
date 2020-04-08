# ChangeLog for Ice

## 0.0.4 - 2020/04/08

### Added

- Methods tellg/tellp, seekg/seekp, good and eof for fileobject
- Support enum { \[ident \[: integer\]\]* }

### Changed

- Coroutine as a library now rather than the example
- Suffix from .ice to .icec

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

- This ChangeLog file to document changes of Ice

### Removed

- The origin ChangeLog in README.md
- The Samples.md, because I plan to write some docs later and you can learn something about how to use Ice by examples in [example/](example/)

## 0.0.0

### Others

- Groundbreak! Everything already exists!
