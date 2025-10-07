[BITS 32]
[ORG 0x9800]

jmp start

KERNEL_PROPERLY_LOADED: db "[KERNEL] Loaded Successfully: KERNEL", 10
PRINTSYS_PROPERLY_LOADED: db "[KERNEL] Loaded Successfully: PRINTSYS", 10
PRINTK_PROPERLY_LOADED: db "[KERNEL] Loaded Successfully: PRINTK", 10
EXCEPTIONS_PROPERLY_LOADED: db "[KERNEL] Loaded Successfully: EXCEPTIONS", 0

%include "kernel/modules/printsys.inc"
%include "kernel/modules/idt.inc"
%include "kernel/modules/exceptions.inc"

start:
    call init_idt
    call init_cpu_exceptions
    call loadidt
    sti
    mov bh, 0x0F
    call clearScreen

    ;mov bh, 0x0F
    call printint

    ;call [tmp2]
    printk KERNEL_PROPERLY_LOADED, 0x0F
    mov eax, printsys
    hlt



hang:
    jmp hang
tmp2: dq 0, 0, 0, 0, 0, 0
tmp: dd 0, 0, 0, 0, 0, 0, 0