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
%include "kernel/modules/blk/ata.inc"
%include "kernel/modules/fs/rwfs.inc"

start:
    call init_idt
    call init_cpu_exceptions
    call ATA_INIT
    call loadidt
    sti
    mov bh, 0x0F
    call clearScreen

    printk KERNEL_PROPERLY_LOADED, 0x0F
    mov eax, 5
    call rwfs_init

times 1024 nop
times 1024 add eax, 0

hang:
    call printstate
    jmp hang

tmp: dd 0, 0, 0, 0, 0, 0, 0