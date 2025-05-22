#!/bin/bash
set -e

echo "[+] Assembling Multiboot header..."
nasm -f elf multiboot_header.asm -o multiboot_header.o

echo "[+] Compiling kernel..."
i686-linux-gnu-g++ -ffreestanding -m32 -c kernel.cpp -o kernel.o

echo "[+] Linking kernel..."
i686-linux-gnu-ld -m elf_i386 -T link.ld -o kernel.bin multiboot_header.o kernel.o

echo "[+] Preparing ISO structure..."
mkdir -p isodir/boot/grub
cp kernel.bin isodir/boot/

echo "[+] Writing GRUB config..."
cat > isodir/boot/grub/grub.cfg <<EOF
menuentry "cOS2 Kernel" {
    multiboot /boot/kernel.bin
}
EOF

echo "[+] Creating ISO..."
grub-mkrescue -o cOS2.iso isodir

echo "[+] Done. Run it with:"
echo "    qemu-system-i386 -cdrom cOS2.iso"
