;========================
; idt.inc
;========================
; 32-bit IDT setup
;------------------------

BITS 32

;------------------------
; IDT table
; 256 entries, 8 bytes each
;------------------------
align 8
IDT:
times 256 dq 0

IDTDescriptor:
    dw 0          ; limit (size-1)
    dd 0          ; base address

;------------------------
; setidt: set one IDT entry
; eax = handler address
; bl  = index (0..255)
;------------------------
setidt:
    push ebx
    push ecx
    push edx

    movzx bx, bl
    shl ebx, 3            ; entry size = 8 bytes → offset in table

    mov word [IDT + ebx + 0], ax         ; lower 16 bits of offset
    shr eax, 16
    mov word [IDT + ebx + 6], ax         ; upper 16 bits of offset
    ; set selector (code segment)
    mov word [IDT + ebx + 2], 0x08
    ; type/flags: 0x8E = present, ring0, 32-bit interrupt gate
    mov byte [IDT + ebx + 5], 0x8E
    mov byte [IDT + ebx + 4], 0         ; zero byte

    pop edx
    pop ecx
    pop ebx
    ret

;------------------------
; loadidt: load IDT
;------------------------
loadidt:
    lea eax, [IDTDescriptor]
    mov word [eax], 2047           ; 256*8-1 = 2047
    lea edx, [IDT]
    mov [eax+2], edx               ; base
    lidt [eax]
    ret

;------------------------
; SIDT macro (store IDT)
;  usage: SIDT dest
;  dest = 6-byte structure: dw limit, dd base
;------------------------
%macro SETIDT 2
    mov eax, %1
    mov bl, %1
    call setidt
%endmacro

default_stub_int:
    cli
    pushad
    pushfd

    ; --- optional: do something for the IRQ ---

    ; send EOI
    cmp bl, 40h        ; IRQ number? (example for IRQ 8–15)
    jb .master_only
    mov al, 0x20
    out 0xA0, al       ; EOI to slave
.master_only:
    mov al, 0x20
    out 0x20, al       ; EOI to master

    popfd
    popad
    sti
    iret



init_idt:
    mov cl, 255
.tmp:
    mov eax, default_stub_int
    mov bl, cl
    call setidt
    cmp cl, 0
    jz .end
    dec cl
    jmp .tmp
.end:
    ret