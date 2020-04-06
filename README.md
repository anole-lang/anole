# Ice Programming Language

[![New Issue](https://img.shields.io/badge/request-new%20features-blue.svg)](https://github.com/ice-lang/ice/issues/new)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/ice-lang/ice/compare)
[![Gitter](https://badges.gitter.im/JoinChat.svg)](https://gitter.im/ice-lang)
[![License](https://img.shields.io/github/license/MU001999/ice.svg)](https://github.com/ice-lang/ice)

## Quick Usage

### Install

```bash
~> git clone https://github.com/ice-lang/ice-lang.git
~> cd ice-lang
~/ice-lang> mkdir build
~/ice-lang> cd build
~/build> cmake ..
~/build> make
~/build> sudo make install
```

If you want to remove ice, you can execute `cat install_manifest.txt | sudo xargs rm` in `build/`

### Usage

```bash
~> ice
```

You can see some examples in `example/` or the `test/sample-tester.cpp`, this is the yin-yang puzzle for fun

```
(@(yang): @(yin): yin(yang))
    ((@(cc) { print("*"); return cc; })
        (call_with_current_continuation(@(cont): cont)))
    ((@(cc) { print("@"); return cc; })
        (call_with_current_continuation(@(cont): cont)));
```

### Extension in Visual Studio Code

Search `Ice-Lang`, only provides highlight now

## ChangeLog

See [ChangeLog.md](ChangeLog.md)

## To Do

Ice-lang is segrageted to about three parts, the front end, codegen and the virtual machine

+ [ ] Useful error info when meeting runtime error, such as traceback like python
+ [ ] Support type convertion
+ [ ] Support threads and processes
+ [ ] Complete all codegens
+ [ ] Read from file and complie IR to file
