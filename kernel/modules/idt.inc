; =====================================
; idt.inc — flat 32-bit IDT + stubs
; Protected mode safe version
; =====================================

BITS 32

; ----- IDT table (256 entries, 8 bytes each)

IDT_BASE:
    times 256 dq 0
IDT_END:

; ----- IDT pointer (6 bytes)
IDTPTR:
    dw IDT_END - IDT_BASE - 1   ; limit
    dd IDT_BASE                 ; base

isr_stub_str: db "[KERNEL] Stub: ", 0
; ------------------------------------
; SETIDT — sets one entry in the IDT
; AL  = entry number (0–255)
; EBX = handler address
; ------------------------------------
SETIDT:
    push edx                  ; save scratch

    movzx edx, al             ; vector number -> EDX
    shl edx, 3                ; 8 bytes per entry
    lea edi, [IDT_BASE + edx] ; EDI = &IDT[vector]

    mov eax, ebx              ; handler address
    mov word [edi], bx        ; offset low 16 bits
    mov word [edi+2], 08h     ; code segment selector
    mov byte [edi+4], 0       ; reserved
    mov byte [edi+5], 8Eh     ; type/attr (present, ring0, 32-bit int gate)
    shr eax,16
    mov word [edi+6], ax      ; offset high 16 bits

    pop edx
    ret


; ------------------------------------
; Default ISR stub
; ------------------------------------
global isr_stub
isr_stub:
    call int_to_char
    clearScreen 0x0F
    printString 0, 0, isr_stub_str, RED*FOREGROUND+BLACK*BACKGROUND
    printString 15, 0, output_of_int_to_char,RED*FOREGROUND+BLACK*BACKGROUND
    ; optional: print vector or debug info here
    cli
.hang:
    hlt
    jmp .hang

; ------------------------------------
; LOADIDT — load IDT into CPU
; ------------------------------------
LOADIDT:
    lidt [IDTPTR]
    ret

; ------------------------------------
; Macros
; ------------------------------------

%macro IDTDEF 2
    mov al, %1        ; vector number
    mov ebx, %2       ; handler
    call SETIDT
%endmacro

%macro IDTINIT 0
    ; ---------------------------
    ; CPU exceptions 0–31
    ; ---------------------------
    push eax
    push ebx
    mov al, 0
%%lloop:
    mov ebx, isr_stub
    call SETIDT
    inc al
    cmp al, 31
    jbe %%lloop
    pop ebx
    pop eax
%endmacro