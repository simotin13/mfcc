#!/bin/bash
cd ../src
make
cd ../test
cp ../src/gpcc .
./gpcc test.c
nasm -f elf64 ./tmp.s
gcc ./tmp.o
./a.out
echo $?
