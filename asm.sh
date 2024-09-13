#!/usr/bin/bash

# gcc -g3 ./main.c -Dcompile_to_x86_64_asm -o main
# ./main test.b
# "-g -F dwarf" enables debugging
# nasm -f elf -g -F dwarf bf.asm && gcc -m32 -o bf bf.o

# echo -e "\n\nRunning\n\n"

# ./bf


nasm -f elf -g -F dwarf bf-int.asm && gcc -m32 -o bf-int bf-int.o && ./bf-int
