nasm -f elf64 tmp.s
gcc tmp.o
objdump -D a.out -M intel > chk.txt

