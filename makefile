all: build/kernel.bin

build/kernel.bin: boot/bootloader.asm kernel/asm/entry.asm kernel/kernel.c kernel/file_system.c kernel/mystdlib.c kernel/cli.c kernel/auth.c
	mkdir -p build
	nasm boot/bootloader.asm -f bin -o build/boot.bin
	nasm kernel/asm/entry.asm -f elf -o build/entry.o
	gcc -m32 -ffreestanding -fno-pic -fno-pie -c kernel/kernel.c -o build/kernel.o
	gcc -m32 -ffreestanding -fno-pic -fno-pie -c kernel/file_system.c -o build/file_system.o
	gcc -m32 -ffreestanding -fno-pic -fno-pie -c kernel/mystdlib.c -o build/mystdlib.o
	gcc -m32 -ffreestanding -fno-pic -fno-pie -c kernel/cli.c -o build/cli.o
	gcc -m32 -ffreestanding -fno-pic -fno-pie -c kernel/auth.c -o build/auth.o
	ld -m elf_i386 -Ttext 0x1000 build/entry.o build/kernel.o build/file_system.o build/auth.o build/cli.o build/mystdlib.o -o build/kernel.bin --oformat binary

os-image.img: build/kernel.bin build/boot.bin
	dd if=/dev/zero of=os-image.img bs=512 count=2048
	dd if=build/boot.bin of=os-image.img conv=notrunc
	dd if=build/kernel.bin of=os-image.img bs=512 seek=1 conv=notrunc

set:
	sudo modprobe nbd max_part=16

run%: os-image.img
	sudo qemu-nbd --disconnect /dev/nbd$* || true
	sudo qemu-nbd --connect=/dev/nbd$* --format=raw os-image.img
	sudo qemu-system-i386 -drive file=/dev/nbd$*,format=raw,if=ide,cache=none

clean:
	rm -rf build os-image.img
