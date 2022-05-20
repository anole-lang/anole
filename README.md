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
git clone https://github.com/anole-lang/anole.git && cd anole
cmake -S . -B build && cmake --build build -j4
cd build && sudo make install
```

If you want to remove anole, you can execute `cat install_manifest.txt | sudo xargs rm` in `build/`

### Test

Run `cmake -D CMAKE_BUILD_TYPE=Test -S . -B build && cmake --build build -j4`

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
