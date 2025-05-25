;https://wiki.osdev.org/Interrupt_Service_Routines#Compiler_Specific_Interrupt_Directives
; filename: isr_wrapper.s

extern asm_int_handler
extern boot_thread
extern need_init 
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

    mov r11, rsp
    call try_init

    ; check if context switch is needed
    mov rax, [glbl_thread_current]
    mov rbx, [glbl_thread_next]
    cmp rax, rbx
    je return_no_context_switch
    ; else...

;//NOTE: if switching, swap the stack context
    mov r11, rsp ; pass stack pointer into function, as calling fn messes with stack
    call save_stack_to_current_thread
    mov r11, rsp ; pass stack pointer into function, as calling fn messes with stack
    call load_next_thread_to_stack

    add rsp, 16
 
    ; record the switch
    mov rax, [glbl_thread_next]
    mov [glbl_thread_current], rax
    ; proceed to next thread
    jmp return_yes_context_switch
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
    pop rax ; not actually restoring rax
    mov es, ax
    pop rax ; not actually restoring rax
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
    pop rax ; actually restoring rax
    ; WARN: assumes that rip, cs, rflags are already in the correct place
    add rsp, 8     ; Remove error code, restoring stack pointer
    iretq

return_yes_context_switch:
;//NOTE: if returning to whence you came, pop off stack
    ; Restore general-purpose registers
    
    pop gs
    pop fs
    pop rax ; not actually restoring rax
    mov es, ax
    pop rax ; not actually restoring rax
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
    pop rax ; actually restoring rax
    ; WARN: assumes that rip, cs, rflags are already in the correct place
    iretq
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
save_stack_to_current_thread:

    mov rbx, [glbl_thread_current] ; reg <-(pointer) [label]
    add rbx, 8          ; reg <-(pointer.context)[pointer + 8]

.copy_stack:
    ; copy the stack to the context struck, from gs to rflags (no rsp/ss)
    %assign z 0
    %rep 22
        mov rax, [r11 + 8*(z+1)] ; read stack
        mov [rbx + 8*(24-z)], rax ; place in struct

        %assign i i+1
        %assign z z+1
    %endrep

.check_if_user_proc:
    ; account for conditional push of RSP and SS
    ; assumes isr is always ring 0
    mov rax, [rbx + 32]; [CS_static_var]
    and rax, 0x3
    cmp rax, 0
    jne .thread_was_not_kernel
.was_kernel:
    ; //WARN: dummy ss and rsp
    mov qword [rbx + 16], rsp; [RSP_static_var]
    mov qword [rbx + 8], ss; [SS_static_var]
    jmp .check_done
.thread_was_not_kernel:
    ; save SS and RSP, as they should have been pushed to stack
    mov rax, [r11 + 23*8]; [RSP_static_var]
    mov [rbx + 16], rax
    mov rax, [r11 + 24*8] ; [SS_static_var]
    mov [rbx + 8], rax
    jmp .check_done
.check_done:
    ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
load_next_thread_to_stack:
    ; rbx = &glbl_thread_next->context WARN:
    
    mov rbx, [glbl_thread_next]  ; reg <-(pointer) [label]
    add rbx, 8          ; reg <-(pointer.context)[pointer + 8]

    ; copy the stack to the context struck, from gs to rflags (no rsp/ss)
    %assign z 0
    %rep 24
        mov rax, [rbx + 8*(24-z)] ; access stored
        mov [r11 + 8*(z+1)], rax ; place on stack

        %assign i i+1
        %assign z z+1
    %endrep
    ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
try_init:
    mov rax, [need_init]
    cmp rax, 1
    jne .skip_init

    call save_stack_to_current_thread
    mov rax, 0
    mov [need_init], rax

    .skip_init:
    ret
