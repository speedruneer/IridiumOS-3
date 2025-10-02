[bits 32]
global SETIDT, LOADIDT
jmp IDTEND
START:
IDT_BASE: times 256 dd 0, 0
END:
IDTPTR: dw END-START-1
dd IDT_BASE
;---------------------------
; SETIDT entry, handler
; AL = entry number
; EBX = handler address
; IDT_BASE = base of IDT in memory
;---------------------------
SETIDT:
    push eax
    push edx

    movzx eax, al           ; entry number -> EAX
    shl eax, 3              ; multiply by 8 (size of IDT entry)
    mov edx, IDT_BASE
    add edx, eax            ; EDX = address of IDT entry

    ; set lower 16 bits of handler
    mov ax, bx
    mov [edx], ax

    ; set segment selector (flat CS = 0x08)
    mov word [edx + 2], 0x08

    ; type/attributes = 0x8E (present, DPL0, 32-bit interrupt gate)
    mov byte [edx + 4], 0x8E

    ; zero byte
    mov byte [edx + 5], 0

    ; upper 16 bits of handler
    shr ebx, 16
    mov word [edx + 6], bx

    pop edx
    pop eax
    ret

LOADIDT:
    lidt [IDTPTR]

%macro IDTDEF 2
    push al
    push ebx
    mov al, %1
    mov ebx, %2
    call SETIDT
    pop ebx
    pop al
%endmacro

%macro LIDT 0
    call LOADIDT
%endmacro

IDTEND: