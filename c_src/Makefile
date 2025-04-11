
# compile and link flags from https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
CFLAGS = -Wall -Werror -Wextra -g -ffreestanding -mno-red-zone -fno-exceptions #-fno-rtti
CFLAGS += -fno-stack-protector
LFLAGS = -nostdlib -lgcc

# List all source files here
#SRCS = src/kernel_main.c src/sealib.c src/vgalib.c
#OBJS = $(SRCS:.c=.o)

# Name of the output binary
# OUTPUT = kernel_packed.o

all: 
# Rule to compile .c files into .o files
	gcc $(CFLAGS) -c src/printlib.c -o src/printlib.o
	gcc $(CFLAGS) -c src/sealib.c -o src/sealib.o
	gcc $(CFLAGS) -c src/vgalib.c -o src/vgalib.o
	gcc $(CFLAGS) -c src/kernel_main.c -o src/kernel_main.o

## Rule to create the final binary by linking the object files
#$(OUTPUT): $(OBJS)
#	gcc $(LFLAGS) $(OBJS) -o $(OUTPUT)
#
#	ar rcs  

# Clean up the build directory and object files
clean:
	rm -rf build $(OBJS) $(OUTPUT)

