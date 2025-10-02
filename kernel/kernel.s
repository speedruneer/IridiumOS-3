[bits 16]
[org 0x9000]

mov si, kernel_loaded
call print_string

hang:
    jmp hang

print_string:
    pusha
.next_char:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    jmp .next_char
.done:
    popa
    ret

kernel_loaded: db "Kernel Loaded"