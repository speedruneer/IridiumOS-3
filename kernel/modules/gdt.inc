;=============================================================================
; gdt_flat.s - Simple flat memory model GDT
; Usage: %include "gdt_flat.s"
;=============================================================================

[bits 32]

; ------------------------------
; GDT descriptor structure
; ------------------------------
gdt_start:
    ; NULL descriptor
    dd 0x0
    dd 0x0

    ; Code segment: base=0, limit=4GB, 0x9A = code segment, 0xCF = granularity
    dw 0xFFFF                  ; limit low
    dw 0x0000                  ; base low
    db 0x00                    ; base middle
    db 10011010b               ; access byte: present, ring 0, code, executable, readable
    db 11001111b               ; flags: 4 KB granularity, 32-bit
    db 0x00                    ; base high

    ; Data segment: base=0, limit=4GB, 0x92 = data segment, 0xCF = granularity
    dw 0xFFFF                  ; limit low
    dw 0x0000                  ; base low
    db 0x00                    ; base middle
    db 10010010b               ; access byte: present, ring 0, data, writable
    db 11001111b               ; flags: 4 KB granularity, 32-bit
    db 0x00                    ; base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; limit
    dd gdt_start               ; base
