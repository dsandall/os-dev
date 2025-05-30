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

BOOT_OBJS := $(shell find ./build -name '*.o')
C_OBJS := $(shell find ./c_src/build -name '*.o')

# Default target
all: clean link
	@printf "$(INFO) Copying grub.cfg, linking grub code, and building ISO with grub-mkrescue...\n"
	cp src/grub.cfg build/isofiles/boot/grub
	make build_fat_img

#build_iso:
#	grub-mkrescue -o build/os.iso build/isofiles
#	@printf "$(INFO) Running OS...\n"
#	qemu-system-x86_64 -s -cdrom build/os.iso

build_fat_img:
	fish --init-command='set fish_trace on' createfat32img.fish
	@printf "$(INFO) Running OS...\n"
	qemu-system-x86_64 \
		-m 256M\
		-drive format=raw,file=build/fat32.img\
		-S -s\
		-serial stdio
		#-d int,cpu_reset \

link: boot
	@printf "$(LINK) Linking into kernel.bin...\n"
	$(MAKE) -C c_src all
	ld --nmagic -nostdlib \
		--output build/isofiles/boot/kernel.bin \
		--script src/linker.ld \
		$(BOOT_OBJS) $(C_OBJS)
	@#printf "$(INFO) Section headers:\n"
	@#objdump -h kernel.bin

boot: src/boot.asm src/longboot.asm src/multiboot_header.asm src/isr_wrapper.s
	@printf "$(BUILD) Assembling multiboot header...\n"
	@printf "$(BUILD) Assembling bootcode called by grub...\n"
	@printf "$(BUILD) Assembling longmode boot code called by 32b bootcode...\n"
	nasm -f elf64 -o build/multiboot_header.o src/multiboot_header.asm
	nasm -f elf64 -o build/boot.o src/boot.asm
	nasm -f elf64 -o build/longboot.o src/longboot.asm
	nasm -f elf64 -o build/isr_wrapper.o src/isr_wrapper.s
	@# nasm boot.asm
	@# hexdump -x boot
	@# ndisasm -b 32 boot

clean:
	@printf "$(CLEAN) Removing build files...\n"
	rm -fr build
	mkdir --parents build/isofiles/boot/grub

