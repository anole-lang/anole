# Anole Programming Language

[![New Issue](https://img.shields.io/badge/request-new%20features-blue.svg)](https://github.com/anole-lang/anole/issues/new)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/anole-lang/anole/compare)
[![Gitter](https://badges.gitter.im/JoinChat.svg)](https://gitter.im/anole)
[![License](https://img.shields.io/github/license/anole-lang/anole.svg)](https://github.com/anole-lang/anole)

## Quick Usage

### Requirements

```bash
sudo apt-get install libreadline6-dev
```

### Install

```bash
~> git clone https://github.com/anole-lang/anole.git
~> cd anole
~/anole> mkdir build
~/anole> cd build
~/build> cmake ..
~/build> sudo make install
```

If you want to remove anole, you can execute `cat install_manifest.txt | sudo xargs rm` in `build/`

### Usage

```bash
~> anole
```

You can see some examples in `example/` or the `test/sample-tester.hpp`, this is the yin-yang puzzle for fun

```
(@(yang): @(yin): yin(yang))
    ((@(cc) { print("*"); return cc; })
        (call_with_current_continuation(@(cont): cont)))
    ((@(cc) { print("@"); return cc; })
        (call_with_current_continuation(@(cont): cont)));
```

### Extension in Visual Studio Code

Search `Anole-Lang`, only provides highlight now

## ChangeLog

See [ChangeLog.md](ChangeLog.md)

## To Do

+ [ ] Support multi return-values and syntax like `return expr1, ..., exprn;` and `@var1, ..., &varn: foo(...);`
+ [ ] Implement self any, variant and so on instead of using the standard library
+ [ ] Use GC instead of simple shared_ptr
+ [ ] Provide debugger
+ [ ] Support multi threads and processes
