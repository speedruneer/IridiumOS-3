bits 32
; =====================================
; irq.inc — IRQ0–IRQ15 stubs
; Flat memory, works with your existing IDT.inc
; =====================================

; --- simple stub for IRQs ---
irq_stub:
    ; End Of Interrupt (EOI) for PIC
    ; send to master or slave as needed
    pushad
    cmp al, 8
    jb .master
    ; slave PIC EOI
    mov al, 0x20
    out 0xA0, al
.master:
    mov al, 0x20
    out 0x20, al
    popad
    iret

%macro IRQINIT 0
    mov al, 32
    mov ebx, isr_stub
%%.tmp_loop_o:
    call SETIDT
    inc al
    cmp al, 48
    jb %%.tmp_loop_o
%endmacro

; ------------------------------------
; PIC remap routine (master 0x20, slave 0x28)
; ------------------------------------
PIC_REMAP:
    ; Save masks
    in al, 0x21
    push eax
    in al, 0xA1
    push eax

    ; Start initialization
    mov al, 0x11        ; ICW1: start init
    out 0x20, al
    out 0xA0, al

    ; ICW2: remap vectors
    mov al, 32          ; Master PIC vector base = 0x20
    out 0x21, al
    mov al, 40          ; Slave PIC vector base = 0x28
    out 0xA1, al

    ; ICW3: wiring
    mov al, 4           ; Master has slave on IR2
    out 0x21, al
    mov al, 2           ; Slave cascade identity
    out 0xA1, al

    ; ICW4: 8086 mode
    mov al, 1
    out 0x21, al
    out 0xA1, al

    ; Restore masks
    pop eax
    out 0xA1, al
    pop eax
    out 0x21, al
    ret
