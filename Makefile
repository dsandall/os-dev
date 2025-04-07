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
all: clean link
	@printf "$(INFO) Copying grub.cfg, linking grub code, and building ISO with grub-mkrescue...\n"
	cp src/grub.cfg build/isofiles/boot/grub
	grub-mkrescue -o build/os.iso build/isofiles

run: all
	@printf "$(INFO) Running OS...\n"
	qemu-system-x86_64 -cdrom build/os.iso

link: boot
	@printf "$(LINK) Linking into kernel.bin...\n"
	ld --nmagic --output build/isofiles/boot/kernel.bin --script src/linker.ld build/multiboot_header.o build/boot.o build/longboot.o
	@#printf "$(INFO) Section headers:\n"
	@#objdump -h kernel.bin

boot: src/boot.asm src/longboot.asm src/multiboot_header.asm
	@printf "$(BUILD) Assembling multiboot header...\n"
	@printf "$(BUILD) Assembling bootcode called by grub...\n"
	@printf "$(BUILD) Assembling longmode boot code called by 32b bootcode...\n"
	nasm -f elf64 -o build/multiboot_header.o src/multiboot_header.asm
	nasm -f elf64 -o build/boot.o src/boot.asm
	nasm -f elf64 -o build/longboot.o src/longboot.asm
	@# nasm boot.asm
	@# hexdump -x boot
	@# ndisasm -b 32 boot

clean:
	@printf "$(CLEAN) Removing build files...\n"
	rm -fr build
	mkdir --parents build/isofiles/boot/grub
