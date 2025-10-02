[bits 16]
[org 0x9000]

times 30 nop

mov si, kernel_loaded
call print_string

jmp kernel

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

%defstr KERNEL_NAME_STR KERNEL_NAME
kernel_loaded: db "Loaded Kernel: ", KERNEL_NAME_STR, 0

kernel:
    %include "kernel/modules/gdt.inc"
    lgdt [gdt_descriptor]
    jmp 0x08:protected_mode_entry

[bits 32]
protected_mode_entry:
    ; Set data segment registers
    mov ax, 0x10   ; data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    call stack_setup
    jmp main_kernel

STACK_TOP equ 0x9FFFF       ; e.g., 64 KB stack at 0x90000

stack_setup:
    ; Load data segment selector into stack segment
    mov ax, 0x10            ; data segment selector from GDT
    mov ss, ax

    ; Set stack pointer
    mov esp, STACK_TOP

    ; Optional: clear stack memory (not required)
    xor eax, eax
    mov ecx, STACK_TOP / 4
    mov edi, 0x0
    rep stosd

    ret

main_kernel:
    jmp main_kernel

%include "kernel/modules/ata.inc"

times 1024 dd 0