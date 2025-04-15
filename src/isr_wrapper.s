;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s


extern asm_int_handler

section .text
bits 64

; interrupt wrapper definition
%macro ISR_WRAPPER 1
global isr_wrapper_%1
isr_wrapper_%1:
    ; Save all general-purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    ; push rsp      ; Optional, probably don't do this
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    cld

    mov rax, %1
    push rax
    lea rdi, [rsp]
    call asm_int_handler
    add rsp, 8

    ; Restore general-purpose registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    ; pop rsp      ; Usually skip this
    pop rbx
    pop rdx
    pop rcx
    pop rax

    iretq
%endmacro

; generate 256 different interrupt_wrappers
%assign i 0
%rep 256
    ISR_WRAPPER i
    %assign i i+1
%endrep
