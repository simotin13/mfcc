#!/bin/bash
cd ../src/mfcc
make
cd ../../test
cp ../src/mfcc/mfcc .
./mfcc test.c
nasm -f elf64 ./tmp.s
gcc ./tmp.o
./a.out
echo $?
