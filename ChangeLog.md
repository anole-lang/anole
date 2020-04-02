# ChangeLog for Ice

## Unreleased

### Added

- Keyword `from` to use in `use` stmt
- Instruction `Import`, `ImportPart` and `ImportAll` to support use statement

### Changed

- Update `use ident` to `use (ident [as ident]) [, ident [as ident]]* [from (ident)]` or `use * from (ident)`
- Use statement can import module from .so files

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
