bits 32
jmp _endprintsys
CurrentCursorPos: dw 0
_color: db 0
; type: function
; Process a byte into 2 hex chars
; -> al byte
; <- ax 2 hex chars
byte_to_hex:
    mov ah, al
    shr al, 4
    and ah, 0x0F

    cmp ah, 9
    jle .hdone
    add ah, 7
.hdone:
    add ah, '0'

    cmp al, 9
    jle .ldone
    add al, 7
.ldone:
    add al, '0'
    ret
writeChar:
    mov edi, 0xB8000
    movzx eax, word[CurrentCursorPos]
    shl eax, 1
    add edi, eax
    mov [edi], bl
    mov bh, [_color]
    mov [edi+1], bh
    inc word[CurrentCursorPos]
    ret
clearScreen:
    mov cx, 2001
    mov [_color], bh
    mov bl, " "
.tmp:
    call writeChar
    dec cx
    cmp cx, 0
    jne .tmp
    mov word[CurrentCursorPos], 0
    ret
; type: function
; Print system
; -> esi string pointer
; -> bl text color
printsys:
    mov [_color], bh          ; store color
.next_char:
    lodsb                     ; AL = [ESI], ESI++
    cmp al, 0
    je .done                  ; end of string
    cmp al, 10                ; newline?
    jne .print_char

    ; handle newline
    mov ax, word [CurrentCursorPos]  ; current cursor in char index
    xor dx, dx
    mov cx, 80                        ; screen width
    div cx                             ; AX = row, DX = column
    inc ax                             ; next row
    mov ax, ax
    mov bx, ax
    mov ax, bx
    mul cx                             ; new CurrentCursorPos = row * 80
    mov word [CurrentCursorPos], ax
    jmp .next_char
.print_char:
    mov bl, al
    call writeChar
    jmp .next_char
.done:
    ret

%macro printk 2
    push esi
    push bx
    mov esi, %1
    mov bh, %2
    call printsys
    pop bx
    pop esi
%endmacro

;------------------------
; printint: prints a 32-bit unsigned integer in decimal
; Inputs:
;   eax = integer to print
;   bh  = color attribute
; Uses:
;   ecx, edx, esi, edi, temp stack
;------------------------
; type: function
; Print an integer 32 bit
; -> eax integer to print
printint:
    push eax
    push ebx
    push ecx
    push edx
    push bx          ; save color

    mov ebx, eax     ; value to print
    mov al, bl
    call byte_to_hex
    mov [localtmp+6], ax
    shr ebx, 8
    mov al, bl
    call byte_to_hex
    mov [localtmp+4], ax
    shr ebx, 8
    mov al, bl
    call byte_to_hex
    mov [localtmp+2], ax
    shr ebx, 8
    mov al, bl
    call byte_to_hex
    mov [localtmp+0], ax

    pop bx
    printk localtmp, bh

    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

localtmp: times 8 db 0
    db 10, 0

; type: function
; Halts and prints all CPU registers (general purpose + segments + EFLAGS + EIP)
printstate:
    cli                 ; disable interrupts
.hang:
    push bx             ; save bx since we'll modify it
    push ax
    push cx
    push dx
    push si
    push di
    push bp

    ; Clear the screen a few times (optional)
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen
    call clearScreen

    ; Dump general-purpose registers
    mov eax, eax
    call print_reg_eax
    mov eax, ebx
    call print_reg_ebx
    mov eax, ecx
    call print_reg_ecx
    mov eax, edx
    call print_reg_edx
    mov eax, esi
    call print_reg_esi
    mov eax, edi
    call print_reg_edi
    mov eax, ebp
    call print_reg_ebp

    ; Dump segment registers
    mov ax, cs
    call print_reg_cs
    mov ax, ds
    call print_reg_ds
    mov ax, es
    call print_reg_es
    mov ax, fs
    call print_reg_fs
    mov ax, gs
    call print_reg_gs
    mov ax, ss
    call print_reg_ss

    ; Dump EFLAGS
    pushfd
    pop eax
    call print_reg_eflags

    ; Halt the CPU
    hlt
    jmp .hang

; ==========================
; Example helper macros/functions for printing
; ==========================
print_reg_eax:   printk reg_eax_str, 0x0F
                 call printint
                 ret
print_reg_ebx:   printk reg_ebx_str, 0x0F
                 call printint
                 ret
print_reg_ecx:   printk reg_ecx_str, 0x0F
                 call printint
                 ret
print_reg_edx:   printk reg_edx_str, 0x0F
                 call printint
                 ret
print_reg_esi:   printk reg_esi_str, 0x0F
                 call printint
                 ret
print_reg_edi:   printk reg_edi_str, 0x0F
                 call printint
                 ret
print_reg_ebp:   printk reg_ebp_str, 0x0F
                 call printint
                 ret
print_reg_cs:    printk reg_cs_str, 0x0F
                 call printint
                 ret
print_reg_ds:    printk reg_ds_str, 0x0F
                 call printint
                 ret
print_reg_es:    printk reg_es_str, 0x0F
                 call printint
                 ret
print_reg_fs:    printk reg_fs_str, 0x0F
                 call printint
                 ret
print_reg_gs:    printk reg_gs_str, 0x0F
                 call printint
                 ret
print_reg_ss:    printk reg_ss_str, 0x0F
                 call printint
                 ret
print_reg_eflags:printk reg_eflags_str, 0x0F
                 call printint
                 ret

; ==========================
; Strings for registers
; ==========================
reg_eax_str: db "EAX: ",0
reg_ebx_str: db "EBX: ",0
reg_ecx_str: db "ECX: ",0
reg_edx_str: db "EDX: ",0
reg_esi_str: db "ESI: ",0
reg_edi_str: db "EDI: ",0
reg_ebp_str: db "EBP: ",0
reg_cs_str:  db "CS: ",0
reg_ds_str:  db "DS: ",0
reg_es_str:  db "ES: ",0
reg_fs_str:  db "FS: ",0
reg_gs_str:  db "GS: ",0
reg_ss_str:  db "SS: ",0
reg_eflags_str: db "EFLAGS: ",0



_endprintsys: