#!/bin/sh

set -xe

CFLAGS="-Wall -Wextra -pedantic -std=c99"
SRC="../src/slex.c ../src/sparser.c ./test.c"

cc $CFLAGS $SRC -Iinclude -o test
