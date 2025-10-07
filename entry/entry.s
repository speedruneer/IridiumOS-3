[BITS 16]
[ORG 0x9000]

mov ah, 1
mov cx, 2607h
int 10h

;---------------------------------
; Bootloader start
;---------------------------------
start:
    mov [BOOT_DRIVE], dl

    ; Print loading message
    mov si, LOADING_KERNEL
    call print

    ; Load 32-bit kernel.s into memory at 0x9800
    mov bx, 0x9800        ; offset in ES
    mov al, KERNEL_SECTORS
    mov si, KERNEL_LBA
    mov dl, [BOOT_DRIVE]
    call read_sectors

    cli

    ; Setup GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; Far jump to 32-bit code
    jmp 0x08:pm_start

;---------------------------------
; Read sectors using BIOS int 13h
; SI = starting LBA (simple CHS sector number)
; CX = number of sectors to read
; ES:BX = buffer address
; DL = boot drive
;---------------------------------
read_sectors:
    push ax
    push bx
    push cx
    push dx
    push si

    mov ah, 0x02           ; BIOS read sectors
    mov al, KERNEL_SECTORS ; number of sectors
    mov ch, 0              ; cylinder 0
    mov cl, KERNEL_LBA     ; starting sector (1-based)
    mov dh, 0              ; head 0
    int 0x13
    jc .fail

    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

.fail:
    mov al, ah
    call byte_to_hex
    mov [ERR_READ_TMP], ax

    mov si, ERR_READ
    call print
    hlt
    jmp $


;---------------------------------
; Print null-terminated string
;---------------------------------
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

;---------------------------------
; Convert AL → AX two ASCII hex chars
;---------------------------------
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

;---------------------------------
; Data
;---------------------------------
BOOT_DRIVE: db 0
LOADING_KERNEL: db "Loading Kernel...",0
ERR_READ:       db "Error reading disk: "
ERR_READ_TMP:   db 0,0,"h",0

;;; gdt_start and gdt_end labels are used to compute size

; null segment descriptor
gdt_start:
    dq 0x0

; code segment descriptor
gdt_code:
    dw 0xffff    ; segment length, bits 0-15
    dw 0x0       ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23
    db 10011010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base, bits 24-31

; data segment descriptor
gdt_data:
    dw 0xffff    ; segment length, bits 0-15
    dw 0x0       ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23
    db 10010010b ; flags (8 bits)
    db 11001111b ; flags (4 bits) + segment length, bits 16-19
    db 0x0       ; segment base, bits 24-31

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit)
    dd gdt_start ; address (32 bit)
gdt_end:

;---------------------------------
; 32-bit Protected Mode start
;---------------------------------
[BITS 32]
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000
    ; Jump to kernel
    jmp 0x08:0x9800

times (512 - ($ - start)) db 0