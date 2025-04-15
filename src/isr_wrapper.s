;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s


global isr_wrapper
extern asm_int_handler

section .text
bits 64

isr_wrapper:
    ; Save all general-purpose registers
    push rax
    push rcx
    push rdx
    push rbx
    push rsp        ; Save current RSP (optional, for debugging)
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

    cld             ; Clear direction flag for C ABI compatibility


mov rax, 0x1234
push rax               ; [rsp] = 0x1234
lea rdi, [rsp]         ; pass address of the pushed value
call asm_int_handler
add rsp, 8
    ; Restore all general-purpose registers (in reverse order)
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
    pop rsp         ; Only safe if previously pushed (usually omit this one)
    pop rbx
    pop rdx
    pop rcx
    pop rax

    iretq

