[bits 16]
[org 0x9000]

mov al, 3
mov ah, 0
int 10h
mov ah, 1
mov cx, 2607h
int 10h

call print_string
cli

jmp kernel_in

print_string:
    sti
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
%include "kernel/modules/gdt.inc"

%defstr KERNEL_NAME_STR KERNEL_NAME
%defstr KERNEL_BUILD_DATE_STR KERNEL_BUILD_DATE
kernel_loaded:
    db "Starting Kernel: "
    db KERNEL_NAME_STR
    times (80 - (%strlen(KERNEL_NAME_STR) + 17)) db ' '
    db "build date: "
    db KERNEL_BUILD_DATE_STR, 0

kernel_in:
    mov si, kernel_loaded
    call print_string
    mov ax, cs
    mov ds, ax
    mov ss, ax
    mov esp, 9FC00h
    lgdt [gdt_descriptor]
    mov eax, cr0
    or  eax, 1        ; set PE bit
    mov cr0, eax
    jmp 0x08:protected_mode_entry

[bits 32]
%include "kernel/modules/data_def.inc"
%include "kernel/modules/txtmode.inc"

%include "kernel/modules/idt.inc"
%include "kernel/modules/irq.inc"

%include "kernel/modules/ata.inc"
;%include "kernel/modules/fs/fat.inc"
;%include "kernel/modules/fs/rwfs.inc"

protected_mode_entry:
    ; Set data segment registers
    mov ax, 10h
    mov ds, ax
    mov cs, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 9FC00h
    clearScreen 0x0F
    printString 0, 0, printload, WHITE*FOREGROUND+BLACK*BACKGROUND

printString 0, 1, kernload, WHITE*FOREGROUND+BLACK*BACKGROUND
IDTINIT
IRQINIT
;call PIC_REMAP
call LOADIDT
sti

; todo: RWFS and finish FAT(12/16/32)

kernhang:
    jmp $

times 4096 jmp kernhang