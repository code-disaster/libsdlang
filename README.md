# libsdlang

## About

`libsdlang` is a compact C library to parse [SDLang](http://sdlang.org/) (Simple Declarative Language) text.

Surprisingly I wasn't able to find a C/C++ implementation of this intriguing little format, so I decided to give it a try.

## Usage

> Note: while implementing the grammar is almost done, I just started working on the library API - so it may change, and be extended and documented (, and become awesome) in the future.

### Source code

It's simple. Add `sdlang.c`, `sdlang.inl` and `sdlang.h` to your project.

The provided `CMakeLists.txt` is written to be used with [fips](http://floooh.github.io/fips/index.html), *the friendly CMake wrapper*. It won't work with regular CMake.

There's a Meson build script too, by courtesy of [handicraftsman](https://github.com/handicraftsman).

### Token API

Have a look at `samples/parser.c`. In summary, `sdlang_parse()` takes a stream function as a parameter, and produces a series of callbacks which can be captured with `sdlang_set_emit_token()` or `sdlang_set_emit_functions()`.

You can capture error output `sdlang_set_report_error()`.

### Threading

The parser is thread-safe.

## The nasty details

### Grammar

The SDLang grammar isn't documented particularly well, which opens *some* room for interpretation. Also, this is the first time I've written a [Ragel](http://www.colm.net/open-source/ragel/) state machine, so there might be loopholes in my implementation.

Additionally, I've made some minor adjustments to the SDLang grammar for convenience (or laziness, or my inability to do it right - feel free to chose one):

- For delimiting single-quoted strings, `'` is supported in addition to backticks. For real, who enjoys typing backticks?
- Integers and floats optionally start with a `+` sign. Also, neither the SDLang page nor the VS Code plugin for syntax highlighting care about the existence of negative numbers, so I took the liberty to allow prefixing numbers with the `-` sign, too.
- Integer and float suffixes can be lower case, so both `1.23F` and `1.23f` are supported.
- Floats can also be written in e-notation, for example `-2.34e-5f`.
- 32/64-bit hexadecimal numbers, like `0x1234abcd` or `0xABCD1234FF`, are supported.

Known bugs:

- The end of a node is evaluated rather lazily. Closing brackets and semicolons generate an "end node" token. Non-wrapping newlines do too, but all of them, even for empty lines, which means that there can be multiple "end node" tokens in a row.
- Semicolons to end a node are recognised but not enforced by the parser, which means these two lines are treated the same - basically, a new node name does end the previous node implicitly:
  - `title "some title"; author "John Smith"`
  - `title "some title" author "John Smith"`

Some value formats are not implemented yet:

- date/time formats

### Implementation

`libsdlang` uses the [Ragel State Machine Compiler](http://www.colm.net/open-source/ragel/) to create its parser FSM.

With less than 1.000 lines of code, the generated code is very compact, and has no external dependencies.

The library doesn't allocate *any* memory. By default, it only uses a few hundred bytes of stack memory to store state and buffer input. You can predefine `SDLANG_PARSE_BUFFERSIZE` to increase the buffer size, which is probably only required when using large literals or string values.

The parser uses a small stack frame for parsing nested SDLang blocks. Stack size can be changed by predefining `SDLANG_PARSE_STACKSIZE`. The stack frame **does not grow** - the parser will stop with a `SDLANG_PARSE_ERROR_STACK_OVERFLOW` when it's full.
