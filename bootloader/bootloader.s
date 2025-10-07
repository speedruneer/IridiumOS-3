BITS 16
ORG 0x7C00

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000

    mov [BOOT_DRIVE], dl   ; store boot drive

    ; Load entry.s (assume 2 sectors)
    mov bx, 0x9000         ; segment offset for entry.s
    mov al, 1              ; sectors to load
    mov si, 2              ; starting LBA (first kernel sector)
    call read_sectors
    jc load_fail
    mov dl, [BOOT_DRIVE]

    jmp 0x00:0x9000      ; jump to entry.s

;-------------------------------
; BIOS INT13h read sectors
; AX = sectors to read
; BX = buffer offset in ES
; SI = starting LBA (simple CHS approximation)
; DL = drive
;-------------------------------
read_sectors:
    push ax
    push bx
    push cx
    push dx

    mov ah, 0x02
    mov ch, 0
    mov cl, 2
    mov dh, 0
    int 0x13

    jc .fail
    pop dx
    pop cx
    pop bx
    pop ax
    ret
.fail:
    stc
    pop dx
    pop cx
    pop bx
    pop ax
    ret

load_fail:
    mov al, ah
    call byte_to_hex
    mov [ERR_READ_TMP], ax
    mov si, ERR_READ
    call print
    hlt

print:
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

byte_to_hex:
    mov ah, al
    shr ah, 4
    and al, 0x0F

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

BOOT_DRIVE: db 0
ERR_READ:       db "Error reading disk: "
ERR_READ_TMP:   db 0,0,"h",0

TIMES 510-($-$$) db 0
DW 0xAA55
