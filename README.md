# Anole Programming Language

[![New Issue](https://img.shields.io/badge/request-new%20features-blue.svg)](https://github.com/anole-lang/anole/issues/new)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/anole-lang/anole/compare)
[![Gitter](https://badges.gitter.im/JoinChat.svg)](https://gitter.im/anole)
[![License](https://img.shields.io/github/license/anole-lang/anole.svg)](https://github.com/anole-lang/anole)

## Quick Usage

### Requirements

Install [Rust](https://www.rust-lang.org/tools/install). Anole tracks the latest
stable Rust toolchain.

### Install

```bash
git clone https://github.com/anole-lang/anole.git && cd anole
cargo install --path .
```

To remove Anole, run `cargo uninstall anole`.

### Test

Run `cargo test --all-targets`. Language behavior tests live in
[`tests/compile`](tests/compile); see its README for adding and updating cases.

### Usage

```console
$ anole
```

You can find examples in `example/`. This is the yin-yang puzzle for fun:

```anole
(@(yang): @(yin): yin(yang))
    ((@(cc) { print("*"); return cc; })
        (call_with_current_continuation(@(cont): cont)))
    ((@(cc) { print("@"); return cc; })
        (call_with_current_continuation(@(cont): cont)));
```

### Extension in Visual Studio Code

Search for `Anole-Lang`. It currently provides syntax highlighting.
