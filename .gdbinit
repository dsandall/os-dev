set arch i386:x86-64:intel
symbol-file ./build/isofiles/boot/kernel.bin 
add-symbol-file ./c_src/build/kernel_main.o
add-symbol-file ./build/boot.o
add-symbol-file ./build/longboot.o
add-symbol-file ./build/multiboot_header.o
add-symbol-file ./c_src/build/interrupts/interrupts.o
target remote localhost:1234 
#layout src

source breakpoints.gdb
