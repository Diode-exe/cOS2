; multiboot_header.asm
section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002         ; magic number
    dd 0x00               ; flags (no memory info required)
    dd -(0x1BADB002 + 0x00) ; checksum

section .text
