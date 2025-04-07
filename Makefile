# CSC 454
# Dylan Sandall
#
# https://os.phil-opp.com/multiboot-kernel/

# Color codes
GREEN=\033[0;32m
BLUE=\033[0;34m
RESET=\033[0m

# Tagged print helpers
INFO=$(GREEN)[INFO]$(RESET)
BUILD=$(BLUE)[BUILD]$(RESET)
LINK=$(GREEN)[LINK]$(RESET)
CLEAN=$(BLUE)[CLEAN]$(RESET)

# Default target
all: clean build_dir link
	@printf "$(INFO) Copying grub.cfg, linking grub code, and building ISO with grub-mkrescue...\n"
	cp src/grub.cfg build/isofiles/boot/grub
	grub-mkrescue -o build/os.iso build/isofiles

run: all
	@printf "$(INFO) Running OS...\n"
	qemu-system-x86_64 -cdrom build/os.iso

build_dir:
	mkdir --parents build/isofiles/boot/grub

link: multiboot_header boot
	@printf "$(LINK) Linking into kernel.bin...\n"
	ld --nmagic --output build/isofiles/boot/kernel.bin --script src/linker.ld build/multiboot_header.o build/boot.o
	@#printf "$(INFO) Section headers:\n"
	@#objdump -h kernel.bin

multiboot_header: src/multiboot_header.asm
	@printf "$(BUILD) Assembling multiboot header...\n"
	nasm -f elf64 -o build/multiboot_header.o src/multiboot_header.asm
	@# nasm multiboot_header.asm
	@# hexdump -x multiboot_header

boot: src/boot.asm
	@printf "$(BUILD) Assembling bootcode called by grub...\n"
	nasm -f elf64 -o build/boot.o src/boot.asm
	@# nasm boot.asm
	@# hexdump -x boot
	@# ndisasm -b 32 boot

clean:
	@printf "$(CLEAN) Removing build files...\n"
	rm -fr build

.PHONY: all clean

