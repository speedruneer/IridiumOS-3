bits 16
; ------------------------------
; GDT descriptor structure
; ------------------------------
gdt_start:

; Null
gdt_null:
    dd 0
    dd 0

; Code segment
gdt_code:
    dw 0xFFFF      ; limit low
    dw 0            ; base low
    db 0            ; base mid
    db 0x9A         ; access
    db 0xCF         ; flags + limit high
    db 0            ; base high

; Data segment
gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 0x92
    db 0xCF
    db 0


gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1   ; limit
    dd gdt_start                 ; base
