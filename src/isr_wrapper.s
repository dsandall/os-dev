;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s

extern asm_int_handler
extern cs_static_var

extern RIP_static_var
extern CS_static_var
extern RFLAGS_static_var
extern RSP_static_var
extern SS_static_var

extern DS_static_var
extern ES_static_var

extern glbl_thread_current
extern glbl_thread_next

section .text
bits 64
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; interrupt wrapper definition
%macro ISR_WRAPPER 1
global isr_wrapper_%1
isr_wrapper_%1:

  ; hardware pushes:
  ; SS (only for ring change)
  ; RSP (only for ring change)
  ; RFLAGS
  ; CS
  ; RIP
  ; (and sometimes ERROR_CODE)

  ; ensure stack pushing is consistent for error codes
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
    mov ax, ds
    movzx rax, ax ; ds and es are funky, need to use zero extension
    push rax
    mov ax, es
    movzx rax, ax ; ds and es are funky, need to use zero extension
    push rax
    push fs
    push gs
    
    cld
    mov rdi, %1 ; place interrupt vector in rdi (first function arg reg, sysv calling conventions)
    mov rsi, [rsp + 19*8]    ; 2nd arg: error code (at top of stack before saving context)
    call asm_int_handler
    
    mov rax, [glbl_thread_current]
    mov rbx, [glbl_thread_next]
    cmp rax, rbx
    je return_no_context_switch
    ; else...

;//NOTE: if not returning, save context off the stack
    mov r11, rsp ; pass stack pointer into function, as calling fn messes with stack
    call save_iret_context_to_current_thread
    mov r11, rsp ; pass stack pointer into function, as calling fn messes with stack
    call load_next_thread_to_context_and_return
    jmp return_no_context_switch
%endmacro

; generate 256 different interrupt_wrappers
%assign i 0
%rep 256
    ISR_WRAPPER i
    %assign i i+1
%endrep
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
return_no_context_switch:
;//NOTE: if returning to whence you came, pop off stack
    ; Restore general-purpose registers
    pop gs
    pop fs
    pop rax
    mov es, ax
    pop rax
    mov ds, ax
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
    ; WARN: assumes that rip, cs, rflags are already in the correct place
    add rsp, 8     ; Remove error code, restoring stack pointer
    iretq
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
save_iret_context_to_current_thread:
    ; preemptively view the same things that iretq pops off the stack
    mov rax, [r11 + 20*8]
    mov [RIP_static_var], rax

    mov rax, [r11 + 21*8]
    mov [CS_static_var], rax 

    mov rax, [r11 + 22*8]
    mov [RFLAGS_static_var], rax

    ; account for conditional push of RSP and SS
    ; assumes isr is always ring 0
    mov rax, [CS_static_var]
    and rax, 0x3
    cmp rax, 0
    je .prev_thread_was_ring0

    ; save SS and RSP, as they should have been pushed to stack
    mov rax, [r11 + 23*8]
    mov [RSP_static_var], rax
    mov rax, [r11 + 24*8]
    mov [SS_static_var], rax
    jmp .check_done

.prev_thread_was_ring0:
    ; dummy ss and rsp 
    mov qword [RSP_static_var], 0x0
    mov qword [SS_static_var], 0x0
    jmp .check_done

.check_done:
    ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
load_next_thread_to_context_and_return:
  ret
