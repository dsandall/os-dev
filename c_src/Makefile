
# compile and link flags from https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
CFLAGS = $(shell cat compile_flags.txt | xargs)
LFLAGS = -nostdlib -lgcc

# List all source files here
#SRCS = src/kernel_main.c src/sealib.c src/vgalib.c
#OBJS = $(SRCS:.c=.o)

# Name of the output binary
# OUTPUT = kernel_packed.o

all: clean
# Rule to compile .c files into .o files
	mkdir build
	clang $(CFLAGS) -c src/gxlib/printlib.c -o build/printlib.o
	clang $(CFLAGS) -c src/sealib.c -o build/sealib.o
	clang $(CFLAGS) -c src/gxlib/vgalib.c -o build/vgalib.o
	clang $(CFLAGS) -c src/kernel_main.c -o build/kernel_main.o

## Rule to create the final binary by linking the object files
#$(OUTPUT): $(OBJS)
#	gcc $(LFLAGS) $(OBJS) -o $(OUTPUT)
#
#	ar rcs  

# Clean up the build directory and object files
clean:
	rm -rf build $(OBJS) $(OUTPUT)

