# ChangeLog for Ice

## Unreleased

### Added

- Support history at REPL mode
- Prompt undefined symbol with its name
- Directory `lib/` for standard libraries

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
