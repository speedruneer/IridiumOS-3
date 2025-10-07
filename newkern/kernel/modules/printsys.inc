bits 32
jmp _endprintsys
CurrentCursorPos: dw 0
_color: db 0
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

localtmp: times 8 dq 0
_endprintsys: