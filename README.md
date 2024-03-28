# Barely a Lisp interpreter

Minimal [Lisp] interpreter implementation that I wrote just for fun and to
test some things. It only has two data types: atoms and lists. It does not
support anything useful but is instead just a list processor.

## Design

Only three data types are available:

- Atoms are just pieces of text that are handled as boolean values, numbers
  or strings depending on the context.
- Lists that are handled as lists of data or function calls depending on the
  context.
- Functions that can be either anonymous or callable by name.
- Optional [M-expression] support.

Special atom called `nil` is treated as an empty/missing value as well as falsy
boolean value. In boolean context every other value than `nil` is treated as
truthy value.

## How to compile

Make sure you have [CMake] and C++11 compiler installed.

```shell
$ git clone https://github.com/RauliL/bali.git
$ cd bali
$ git submodule update --init
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

## How to use

Either run the `bali` executable with an path to a file that contains Lisp
source code, or just `bali` without a filename to start [REPL].

## Builtin functions / operators

Numeric: `+`, `-`, `*`, `/`, `=`, `<`, `>`, `<=`, `>=`.

List: `length`, `cons`, `car`, `cdr`, `list`, `append`, `for-each`, `filter`,
`map`.

Boolean: `not`, `and`, `or`, `if`.

Variables: `setq`, `let`.

Functions: `apply`, `defun`, `lambda`, `return`.

Utilities: `quote`, `load`, `write`.

[Lisp]: https://en.wikipedia.org/wiki/Lisp_(programming_language)
[M-expression]: https://en.wikipedia.org/wiki/M-expression
[CMake]: https://www.cmake.org
[REPL]: https://en.wikipedia.org/wiki/Read%E2%80%93eval%E2%80%93print_loop
