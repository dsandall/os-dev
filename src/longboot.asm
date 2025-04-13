; called by boot asm
; first instance of 64 bit code execution

global long_mode_start
extern kernel_main

section .text
bits 64

long_mode_start:
  ; load 0 into all data segment registers (clears some remaining 32 bit flags)
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

  ; print `OKAY` to screen
    mov rax, 0x2f592f412f4b2f4f
    mov qword [0xb8000], rax

  ; here it is!
    call kernel_main

  ; print smudged `OKAY`s to screen
  ;  mov rax, 0x2f592f412f4b2f4f
  ;  mov qword [0xb8000], rax
  ;  mov qword [0xb8004], rax
  ;  mov qword [0xb8008], rax
  ;  mov qword [0xb800F], rax
  ;  hlt
