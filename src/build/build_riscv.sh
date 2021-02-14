#!/bin/bash
# =============================================================================
# build script for Hifive1 Rev.B
# =============================================================================
riscv64-unknown-elf-as -march=rv32imac -mabi=ilp32 tmp.S 
riscv64-unknown-elf-ld a.out -T hifive1_rev_b.lds -o hifive1.elf -A=rv32imac -melf32lriscv

