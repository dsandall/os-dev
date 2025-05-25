; https://os.phil-opp.com/entering-longmode/
global start

; long mode main
extern long_mode_start

; initial kernel page tables and stack
extern kernel_p4_table
extern kernel_p3_table
extern identity_p2_table
extern stack_kernel1

; data passed by grub
extern multiboot_pointer
extern multiboot_magic

section .text
bits 32
start:
  ; initialize stack pointer reg (esp)
  mov esp, stack_kernel1 + 4096

  ; store GRUB data to static location
  mov [multiboot_pointer], ebx
  mov [multiboot_magic], eax

; verify platform
  call check_multiboot
  call check_cpuid
  call check_long_mode

  ; set up page tables and hugepages, set appropriate cpu registers
  call set_up_page_tables
  call enable_paging

  ; https://os.phil-opp.com/entering-longmode/#loading-the-gdt
  ; load the 64-bit GDT and test 64 bit code execution
  lgdt [gdt64.pointer]

  ; longjump (code selector):(section name) 
  ; forces a context switch to flush lgdt
  jmp gdt64.code:long_mode_start
 
  ; this code is never reached
  ; print 'OK' to screen
  mov dword [0xb8000], 0x2f4b2f4f
  hlt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Prints `ERR: ` and the given error code to screen and hangs.
; parameter: error code (in ascii) in al
error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; verify multiboot 2 compliance
 ;https://os.phil-opp.com/entering-longmode/#multiboot-check
  check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret
  .no_multiboot:
    mov al, "0"
    jmp error

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;https://os.phil-opp.com/entering-longmode/#cpuid-check
  check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for com-drive format=raw,file=build/fat32.img -S -s -d int,cpu_reset -serial stdioparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid
    ret
  .no_cpuid:
    mov al, "1"
    jmp error

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;https://os.phil-opp.com/entering-longmode/#long-mode-check
check_long_mode:
    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode
    ret
.no_long_mode:
    mov al, "2"
    jmp error

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; https://os.phil-opp.com/entering-longmode/#set-up-identity-paging
set_up_page_tables:
    ; map first P4 entry to P3 table
    mov eax, kernel_p3_table
    or eax, 0b11 ; present + writable
    mov [kernel_p4_table], eax

    ; map first P3 entry to P2 table
    mov eax, identity_p2_table
    or eax, 0b11 ; present + writable
    mov [kernel_p3_table], eax

;;; Inner Loop: map each P2 entry to a huge 2MiB page
    mov ecx, 0         ; init counter variable
.map_p2_table:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000  ; 2MiB
    mul ecx            ; start address of ecx-th page
    or eax, 0b10000011 ; present + writable + huge
    mov [identity_p2_table + ecx * 8], eax ; map ecx-th entry

    inc ecx            ; increase counter
    cmp ecx, 512       ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table  ; else map the next entry

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; https://os.phil-opp.com/entering-longmode/#enable-paging
;
enable_paging:
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, kernel_p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; reserve bytes for stack_bottom (allocates space for stack)
;               and page table (required for long mode)
; https://os.phil-opp.com/entering-longmode/#creating-a-stack
;;;;;;;section .bss
;;;;;;;align 4096
;;;;;;;p4_table:
;;;;;;;    resb 4096
;;;;;;;p3_table:
;;;;;;;    resb 4096
;;;;;;;p2_table:
;;;;;;;    resb 4096
;stack_bottom:
;    resb 64
;stack_top:

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; https://os.phil-opp.com/entering-longmode/#the-global-descriptor-table
; https://os.phil-opp.com/entering-longmode/#loading-the-gdt
; create new 64 bit GDT, so that we can switch to 64 bit mode (instead of long mode, with 32b compatibility mode)
section .rodata
gdt64:
    dq 0                          ; null descriptor (mandatory)
.code: equ $ - gdt64              ; label to mark the offset of the code segment (selector will be .code)
    ; 64-bit code segment descriptor
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
    ; Explanation:
    ; Bit 43 = executable (type bit)
    ; Bit 44 = readable
    ; Bit 47 = present
    ; Bit 53 = long mode (L bit)
.pointer:
    dw $ - gdt64 - 1              ; limit (size of GDT - 1)
    dq gdt64                      ; base address of the GDT
