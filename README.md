
# ExprToAsm

## Quickstart

To use `exprtoasm`:

```console
$ ./build.sh
$ ./exprtoasm -v "((2 + 3)*5 - 2)/7"
$ gcc output.s -o output
$ ./output
```

To test the parser, add your lex rules in `lex.h`, semantic rules in `grammar.txt`, input in `input.txt`

```console
$ cd parsing-examples/
$ ./build.sh
$ ./test
```