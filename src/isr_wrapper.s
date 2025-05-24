;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s

extern asm_int_handler
extern cs_static_var

extern RIP_static_var
extern CS_static_var
extern RFLAGS_static_var
extern RSP_static_var
extern SS_static_var

section .text
bits 64

; interrupt wrapper definition
%macro ISR_WRAPPER 1
global isr_wrapper_%1
isr_wrapper_%1:

  ; hardware pushes 
  ; SS (only for ring change?)
  ; RSP (only for ring change?)
  ; RFLAGS
  ; CS
  ; RIP
  ; (and sometimes ERROR_CODE)

  ; ensure stack pushing is consistent for error codes
    %if %1 != 8 && %1 != 10 && %1 != 11 && %1 != 12 && %1 != 13 && %1 != 14 && %1 != 17
        push 0   ; Dummy error code for exceptions that don't push one
    %endif

    ; saving rax, as we need RAX for next checks
    push rax

    ; preemptively view the same things that iretq pops off the stack
    mov rax, [rsp + 16]
    mov [RIP_static_var], rax

    mov rax, [rsp + 24]
    mov [CS_static_var], rax 

; account for conditional push of RSP and SS
; assumes isr is always ring 0
    and rax, 0x3
    cmp rax, 0
    je prev_thread_was_ring0_%1

    ; save SS and RSP, as they should have been pushed to stack
    mov rax, [rsp + 40]
    mov [RSP_static_var], rax
    mov rax, [rsp + 48]
    mov [SS_static_var], rax
    jmp check_done_%1

  prev_thread_was_ring0_%1:

    ; dummy ss and rsp 
    mov qword[RSP_static_var], 0x0
    mov qword[SS_static_var], 0x0
    jmp check_done_%1

  check_done_%1:
    mov rax, [rsp + 32]
    mov [RFLAGS_static_var], rax

  ; then we push:
    ; Save all general-purpose registers
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
    ; save needed for coop multitasking
    ;push ds
    ;push es
    push fs
    push gs
    
    cld
    mov rdi, %1 ; place interrupt vector in rdi (first function arg reg, sysv calling conventions)
    mov rsi, [rsp + 15*8]    ; 2nd arg: error code (at top of stack before saving context)
    call asm_int_handler

    ; restore needed for coop multitasking
    pop gs
    pop fs
    ;pop es
    ;pop ds
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
