bits 32

dd usr_functions - $

usr_functions:
    dd 2
    dd prints - $
    dd clear - $

CurrentCursorPos: dw 0
_color: db 0

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

; type: function
; Clear screen (VGA/CGA text mode)
; -> bh color
clear:
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
; Print in a driver
; -> esi string pointer
; -> bl text color
prints:
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