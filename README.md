# Ice Language

[![](https://img.shields.io/badge/request-new%20features-blue.svg)](https://github.com/ice-lang/ice/issues/new)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://github.com/ice-lang/ice/compare)
[![Gitter](https://badges.gitter.im/JoinChat.svg)](https://gitter.im/ice-lang)
[![license](https://img.shields.io/github/license/MU001999/ice.svg)](https://github.com/ice-lang/ice)

In fact, I am trying to rewrite this program now. If you want to use this language to do something, you can visit [this](https://github.com/ice-lang/ice-old-version), a old version with the implementation of interpretor.

## To Do

Ice-lang is segrageted to about three parts, the front end, codegen and the virtual machine. I am planning to implement the front end firstly, then virtual machine and the codegen part finally.

+ [ ] Complete all codegens
+ [ ] Add more tests for the front end
+ [ ] Support default arguments
+ [ ] Support the built-in classes
+ [ ] Read from file and complie IR to file

## Done

+ [X] Support call-by-need
+ [X] Better implementation of REPL without try-throw
+ [X] Simple REPL with try-throw
+ [X] Complete basic things in vm such as function, control flow and some operators
+ [X] Report errors with line info in detail in front
+ [X] Implementation of the front end but without reporting errors with line info in detail
