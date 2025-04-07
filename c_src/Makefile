# compile and link flags from https://wiki.osdev.org/Why_do_I_need_a_Cross_Compiler%3F
CFLAGS = -Wall -Wextra -g -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti
LFLAGS = -nostdlib -lgcc

SRCS = src/kernel_main.c
OBJS = $(SRCS:.c=.o)

all: clean src/kernel_main.o

%.o: %.c
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -rf build

