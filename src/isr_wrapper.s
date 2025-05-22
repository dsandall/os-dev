;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s

extern asm_int_handler

section .text
bits 64

; interrupt wrapper definition
%macro ISR_WRAPPER 1
global isr_wrapper_%1
isr_wrapper_%1:

    %if %1 != 8 && %1 != 10 && %1 != 11 && %1 != 12 && %1 != 13 && %1 != 14 && %1 != 17
        push 0   ; Dummy error code for exceptions that don't push one
    %endif

    ; Save all general-purpose registers
    push rax
    push rcx
    push rdx
    push rbx
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
    ; added for coop
    ;push rsp //TODO:
    ;push rip //TODO:
    ;push rflags //TODO:
    ;push cs
    ;push ss
    ;push ds
    ;push es
    ;push fs
    ;push gs
    
    cld
    mov rdi, %1 ; place interrupt vector in rdi (first function arg reg, sysv calling conventions)
    mov rsi, [rsp + 15*8]    ; 2nd arg: error code (at top of stack before saving context)
    call asm_int_handler

    ; added for coop
    ;push gs
    ;push fs
    ;push es
    ;push ds
    ;push ss
    ;push cs
    ;push rflags //TODO:
    ;push rip //TODO:
    ;push rsp //TODO:
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
    pop rbx
    pop rdx
    pop rcx
    pop rax

    add rsp, 8     ; Remove error code, restoring stack pointer

    iretq
%endmacro

; generate 256 different interrupt_wrappers
%assign i 0
%rep 256
    ISR_WRAPPER i
    %assign i i+1
%endrep
