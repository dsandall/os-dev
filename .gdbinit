set arch i386:x86-64:intel
symbol-file ./build/isofiles/boot/kernel.bin 
add-symbol-file ./c_src/src/kernel_main.o
target remote localhost:1234 
layout src
