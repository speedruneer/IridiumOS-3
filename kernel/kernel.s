[bits 16]
[org 0x9000]

mov al, 3
mov ah, 0
int 10h

jmp kernel_in

print_string:
    pusha
.next_char:
    lodsb
    cmp al, 0
    jz .done
    mov ah, 0x0E
    mov bh, 0
    mov bl, 0x07
    int 0x10
    jmp .next_char
.done:
    popa
    ret

%defstr KERNEL_NAME_STR KERNEL_NAME
kernel_loaded: db "Loaded Kernel: ", KERNEL_NAME_STR, 0

kernel_in:
    mov si, kernel_loaded
    call print_string
    lgdt [gdt_descriptor]
    jmp 0x08:protected_mode_entry

%include "kernel/modules/gdt.inc"

[bits 32]
%include "kernel/defines.inc"
protected_mode_entry:
    ; Set data segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, STACK_TOP
    jmp main_kernel

main_kernel:

%include "kernel/modules/idt.inc"
%include "kernel/modules/ata.inc"
;%include "kernel/modules/fs/fat.inc"
%include "kernel/modules/fs/rwfs.inc"
%include "kernel/modules/txtmode.inc"

; todo: IRQs, Core ISRs, RWFS and finish FAT(12/16/32)

clearScreen 0xF0
;printString 1, 1, printload, BLUE
hang:
mov cl, 1
mov ah, 1
mov esi, printload
mov bh, BLUE
call print
jmp hang

printload: db "[KERNEL] Loaded PRINT successfully", 0
times 1024 dd 0