;========================
; exceptions.inc
;========================
; 32-bit CPU exceptions
; Each exception has its own function and message
;------------------------

BITS 32

global init_cpu_exceptions

;------------------------
; Safe print function
; Clears screen, prints string pointed by ESI with color BH
;------------------------
; type: function
; Prints safely (still uses printsys so unsafe but who cares)
; -> eax input string addr
; -> bh input color
safe_print:
    pushad
    call clearScreen
    call printsys
    popad
    ret

;------------------------
; Exception functions
; Each prints its own message and halts
;------------------------

exception_divide_by_zero:
    cli
    mov esi, msg_divide_by_zero
    mov bh, 0x4F
    call safe_print
.hang_div0:
    hlt
    jmp .hang_div0

exception_debug:
    cli
    mov esi, msg_debug
    mov bh, 0x4F
    call safe_print
.hang_debug:
    hlt
    jmp .hang_debug

exception_nmi:
    cli
    mov esi, msg_nmi
    mov bh, 0x4F
    call safe_print
.hang_nmi:
    hlt
    jmp .hang_nmi

exception_breakpoint:
    cli
    mov esi, msg_breakpoint
    mov bh, 0x4F
    call safe_print
.hang_bp:
    hlt
    jmp .hang_bp

exception_overflow:
    cli
    mov esi, msg_overflow
    mov bh, 0x4F
    call safe_print
.hang_overflow:
    hlt
    jmp .hang_overflow

exception_bound_range:
    cli
    mov esi, msg_bound_range
    mov bh, 0x4F
    call safe_print
.hang_bound:
    hlt
    jmp .hang_bound

exception_invalid_opcode:
    cli
    mov esi, msg_invalid_opcode
    mov bh, 0x4F
    call safe_print
.hang_invalid_opcode:
    hlt
    jmp .hang_invalid_opcode

exception_device_not_available:
    cli
    mov esi, msg_device_not_available
    mov bh, 0x4F
    call safe_print
.hang_device:
    hlt
    jmp .hang_device

exception_double_fault:
    iret
    cli
    mov esi, msg_double_fault
    mov bh, 0x4F
    call safe_print
.hang_double_fault:
    hlt
    jmp .hang_double_fault

exception_coprocessor_segment:
    cli
    mov esi, msg_coprocessor_segment
    mov bh, 0x4F
    call safe_print
.hang_coprocessor:
    hlt
    jmp .hang_coprocessor

exception_invalid_tss:
    cli
    mov esi, msg_invalid_tss
    mov bh, 0x4F
    call safe_print
.hang_invalid_tss:
    hlt
    jmp .hang_invalid_tss

exception_segment_not_present:
    cli
    mov esi, msg_segment_not_present
    mov bh, 0x4F
    call safe_print
.hang_segment:
    hlt
    jmp .hang_segment

exception_stack_segment:
    cli
    mov esi, msg_stack_segment
    mov bh, 0x4F
    call safe_print
.hang_stack:
    hlt
    jmp .hang_stack

exception_general_protection:
    cli
    mov esi, msg_general_protection
    mov bh, 0x4F
    call safe_print
.hang_gp:
    hlt
    jmp .hang_gp

exception_page_fault:
    cli
    mov esi, msg_page_fault
    mov bh, 0x4F
    call safe_print
.hang_pf:
    hlt
    jmp .hang_pf

exception_fpu_fault:
    cli
    mov esi, msg_fpu_fault
    mov bh, 0x4F
    call safe_print
.hang_fpu:
    hlt
    jmp .hang_fpu

exception_alignment_check:
    cli
    mov esi, msg_alignment_check
    mov bh, 0x4F
    call safe_print
.hang_align:
    hlt
    jmp .hang_align

exception_machine_check:
    cli
    mov esi, msg_machine_check
    mov bh, 0x4F
    call safe_print
.hang_machine:
    hlt
    jmp .hang_machine

exception_simd_fpu:
    cli
    mov esi, msg_simd_fpu
    mov bh, 0x4F
    call safe_print
.hang_simd:
    hlt
    jmp .hang_simd

exception_virtualization:
    cli
    mov esi, msg_virtualization
    mov bh, 0x4F
    call safe_print
.hang_virt:
    hlt
    jmp .hang_virt

exception_control_protection:
    cli
    mov esi, msg_control_protection
    mov bh, 0x4F
    call safe_print
.hang_ctrl:
    hlt
    jmp .hang_ctrl

; placeholders for exceptions 22–31
exception_unused_22:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_22:
    hlt
    jmp .hang_22

exception_unused_23:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_23:
    hlt
    jmp .hang_23

exception_unused_24:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_24:
    hlt
    jmp .hang_24

exception_unused_25:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_25:
    hlt
    jmp .hang_25

exception_unused_26:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_26:
    hlt
    jmp .hang_26

exception_unused_27:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_27:
    hlt
    jmp .hang_27

exception_unused_28:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_28:
    hlt
    jmp .hang_28

exception_unused_29:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_29:
    hlt
    jmp .hang_29

exception_unused_30:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_30:
    hlt
    jmp .hang_30

exception_unused_31:
    cli
    mov esi, msg_unused
    mov bh, 0x4F
    call safe_print
.hang_31:
    hlt
    jmp .hang_31

;------------------------
; Messages (zero-terminated)
;------------------------
msg_divide_by_zero:        db "PANIC: Divide by Zero", 0
msg_debug:                 db "PANIC: Debug Exception", 0
msg_nmi:                   db "PANIC: Non-Maskable Interrupt", 0
msg_breakpoint:            db "PANIC: Breakpoint", 0
msg_overflow:              db "PANIC: Overflow", 0
msg_bound_range:           db "PANIC: Bound Range Exceeded", 0
msg_invalid_opcode:        db "PANIC: Invalid Opcode", 0
msg_device_not_available:  db "PANIC: Device Not Available", 0
msg_double_fault:          db "PANIC: Double Fault", 0
msg_coprocessor_segment:   db "PANIC: Coprocessor Segment Overrun", 0
msg_invalid_tss:           db "PANIC: Invalid TSS", 0
msg_segment_not_present:   db "PANIC: Segment Not Present", 0
msg_stack_segment:         db "PANIC: Stack Segment Fault", 0
msg_general_protection:    db "PANIC: General Protection Fault", 0
msg_page_fault:            db "PANIC: Page Fault", 0
msg_fpu_fault:             db "PANIC: x87 FPU Floating-Point Error", 0
msg_alignment_check:       db "PANIC: Alignment Check", 0
msg_machine_check:         db "PANIC: Machine Check", 0
msg_simd_fpu:              db "PANIC: SIMD FPU Exception", 0
msg_virtualization:        db "PANIC: Virtualization Exception", 0
msg_control_protection:    db "PANIC: Control Protection Exception", 0
msg_unused:                db "PANIC: Unused Exception", 0

;------------------------
; Table of handler addresses
;------------------------
exception_table:
    dd exception_divide_by_zero
    dd exception_debug
    dd exception_nmi
    dd exception_breakpoint
    dd exception_overflow
    dd exception_bound_range
    dd exception_invalid_opcode
    dd exception_device_not_available
    dd exception_double_fault
    dd exception_coprocessor_segment
    dd exception_invalid_tss
    dd exception_segment_not_present
    dd exception_stack_segment
    dd exception_general_protection
    dd exception_page_fault
    dd exception_fpu_fault
    dd exception_alignment_check
    dd exception_machine_check
    dd exception_simd_fpu
    dd exception_virtualization
    dd exception_control_protection
    dd exception_unused_22
    dd exception_unused_23
    dd exception_unused_24
    dd exception_unused_25
    dd exception_unused_26
    dd exception_unused_27
    dd exception_unused_28
    dd exception_unused_29
    dd exception_unused_30
    dd exception_unused_31

;------------------------
; Initialize IDT entries 0–31
; Expects: setidt reads handler address in EAX and vector in BL
;------------------------
; type: function
; Initiate CPU Exceptions
init_cpu_exceptions:
    cli
    xor ecx, ecx          ; ecx = 0
.loop:
    cmp ecx, 32
    jge .done
    mov eax, [exception_table + ecx*4]  ; handler address
    mov bl, cl            ; vector number
    call setidt
    inc ecx
    jmp .loop
.done:
    ret
