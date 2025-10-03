[bits 16]
[org 0x7C00]

; ------------------------------
; Variables
; ------------------------------

; Messages

; Stack setup
STACK_SEG equ 0x0000
STACK_SIZE equ 0x1000     ; 4 KB stack

; ------------------------------
; Bootloader start
; ------------------------------
start:
    cli                     ; Disable interrupts

    ; ------------------------------
    ; Setup segments
    ; ------------------------------
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ax, STACK_SEG
    mov ss, ax
    mov sp, STACK_SIZE      ; Stack grows downward

    ; Save boot drive
    mov [BOOT_DRIVE], dl

    ; ------------------------------
    ; Set video mode
    ; ------------------------------
    %ifdef KERNEL_VGA
        mov ah, 0
        %warning "VGA Mode Is Defined?"
        mov al, 13h            ; 320x200x256
        int 0x10
    %endif

    ; ------------------------------
    ; Print loading message
    ; ------------------------------
    mov si, msg_loading
    call print_string

    ; ------------------------------
    ; Load kernel to 0x9000
    ; ------------------------------
    mov si, RETRIES
load_kernel:
    mov ch, 0               ; Cylinder 0
    mov cl, 2               ; Start after bootloader
    mov dh, 0               ; Head 0
    mov dl, [BOOT_DRIVE]
    mov bx, 0x9000          ; Offset
    xor ax, ax
    mov es, ax              ; Segment
    mov ah, 0x02            ; BIOS read sectors
    mov al, KERNEL_SECTORS
    int 0x13
    jnc kernel_success
    jc kernel_error
    kernel_success:
    mov si, msg_loaded
    call print_string
    jmp 0x0000:0x9000       ; Jump to kernel

kernel_error:
    dec si
    jz disk_fail
    jmp load_kernel

disk_fail:
    mov si, msg_error
    call print_string
    .hang:
    jmp .hang

; ------------------------------
; Print string function
; Input: DS:SI -> null-terminated string
; ------------------------------
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

BOOT_DRIVE: db 0
RETRIES:    db 3
msg_loading: db 'Loading kernel...                                                              ',0
msg_loaded: db 'Kernel Loaded!...                                                               ',0
msg_error:   db 'Disk read error!',0
; ------------------------------
; Boot signature
; ------------------------------
times 510-($-$$) db 0
dw 0xAA55