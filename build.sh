#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -pedantic -std=c99"
SRC="./src/slex.c ./src/sparser.c ./src/exprtoasm.c"

cc $CFLAGS $SRC -Iinclude -o exprtoasm
