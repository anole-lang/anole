# ChangeLog for Anole

## Unreleased

## 0.0.19 - 2020/09/02

### Added

- Support block comment by using a pair of '$'

### Updated

- Context uses clearer interfaces `push*/pop*`

### Changed

- Modify the directory structure of Anole

### Fixed

- Fix bug about GC when calling function-like objects

## 0.0.18 - 2020/08/20

### Added

- Anole has GC now!!!
- Built-in function `type` to get the type literal of the given value

### Updated

- Default `=` and `!=` will compare address of given objects directly

### Fixed

- Custom objects in module can add their own types and the corresponding literals

## 0.0.17 - 2020/08/07

### Added

- Add magic number in the head of .ir file to stand for the version of IR
- Add command-line argument `--version` to print the version of anole

## 0.0.16 - 2020/07/28

### Added

- Support use direct path of module like `use * from "/path/to/module"`, `use "/path/to/module" as mod` and `use part as alias from "/path/to/module"`
- Instructions `ImportPath/ImportAllPath` to use module with its path directly
- Instructions `ReturnAc/Return` to hold all return situations
- Instruction `FastCall` for call expression without arguments
- Instruction `ReturnNone` for implicit return none

### Updated

- Dict will create an empty target if the given key is not recorded

### Changed

- Rename Instructions `CallExAnchor/CallEx` to `CallAc/Call`

### Removed

- Remove old Instructions `Call/CallTail/CallExTail/Return` because call expression may return multi values and there are many complex situation

## 0.0.15 - 2020/07/17

### Added

- Support multi ret-vals such as `return expr1, ..., exprn`, `@(): expr1, ..., exprn`
- Supoprt multi vars-decl like `@var1, ..., varn` or `@var1, ..., varn: expr1, ..., exprn`
- Multi ret-vals will be flated on where the call expr appears. For example, `return ..., foo(...)` is equivalent to `return ..., expr1, ..., exprn` after `foo(...)` is called. The call expr with multi ret-vals also can appear in multi vars-decl and as argument of other function, and then it will be flated also
- Enale prefix operator receiving call expr with multi-ret-vals like `cstm_preop foo(...)` with `foo(...): expr1, ..., exprn`

### Changed

- Ins `Return` has an oprand for the number of values to return now
- Enable packed parameter be reference as `...&name`, notice that it is an old feature and just not mention before

## 0.0.14 - 2020/06/17

### Updated

- Use gtest instead of the old simple test to make test
- Builtin function will get the number of arguments when it is called
- Paramemter without default value cannot follow parameter with default value
- Packed parameter should be the last parameter in function declaration
- Update grammar for packed parameter from `name...` to `...name`

### Added

- Instructions `CallEx/CallExTail` for call with unpacked argument
- Add check when calling with less arguments

### Changed

- Instruction `CallAnchor` is renamed to `CallExAnchor`
- Instructions `Call/CallTail` have a oprand for the number of arguments
- No default value for parameter which is not given now, it is none in the past

### Fixed

- Packed parameter will be empty list if it is not given arguments
- Packed parameter cannot have default value now

## 0.0.13 - 2020/06/11

### Updated

- Delay expression can store its computed value now

### Changed

- Use `dict {}` instead of '{}' for dict expr, '{}' will be considered as lambda expression with no parameters
- Support variadic parameters and arguments with `...`
- Call/CallTail without arguments number now

### Added

- New insturctions `Pack/Unpack` for variadic parameters and arguments
- New instruction `ThunkOver` to support storing computed value for delay expression

## 0.0.12 - 2020/06/05

### Added

- Support use list in foreach statement directly
- Method to_str for integer object

### Fixed

- Fix bug that old eval function which will skip the first opcode
- Fix bug in parsing user defined unary operators

### Changed

- Binary operators are left-associative instead of right-associative now
- Built-in method will contain a shared pointer of its object

## 0.0.11 - 2020/04/24

### Added

- Insturction AddPrefixOp to make identifier as prefix operator
- Support custom unary operators now

### Updated

- Rewrite some code to make anole faster

### Changed

- Use ObjectType instead of dynamic_cast to distinguish different objects
- Default compile mode is static now

### Fixed

- Context can traceback to the correct origin when meeting compile-error in REPL mode

### Removed

- Instruction ScopeEnd
- Remove support of block scope new :(

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
