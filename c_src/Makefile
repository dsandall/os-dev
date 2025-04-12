
# compile and link flags from https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
CFLAGS = $(shell cat compile_flags.txt | xargs)
LFLAGS = -nostdlib -lgcc

# List all source files here
#SRCS = src/kernel_main.c src/sealib.c src/vgalib.c
#OBJS = $(SRCS:.c=.o)

# Find all .c files recursively in src/
SRC := $(shell find src -name '*.c')

# Replace src/ with build/ and .c with .o
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

# Default target
all:  clean $(OBJ)

# Pattern rule to compile .c -> .o
build/%.o: src/%.c
	@mkdir -p $(dir $@)
	clang $(CFLAGS) -c $< -o $@


clean:
	rm -rf build
	mkdir build

