%ifdef KERNEL_USE_ATA
[bits 32]
IDTDEF 48, ATA_DISK_SERVICE
LOADIDT
; bl = function (0=read,1=write), params: eax, cl, edi/esi
ATA_DISK_SERVICE:
    cmp bl, 1
    jz .read
    je .write
    ret
.read:
    call ATA_LBA_READ
    ret
.write:
    call ATA_LBA_WRITE
    ret


jmp ATAEND
;=============================================================================
; ATA read sectors (LBA mode, 32-bit protected mode)
;
; Input:
;   EAX = Logical Block Address (LBA)
;   CL  = Number of sectors to read (1-255)
;   EDI = Buffer address to store data
; Output:
;   None
;=============================================================================
ATA_LBA_READ:
    pushfd
    push eax
    push ebx
    push ecx
    push edx
    push edi

    mov ebx, eax            ; Save LBA in EBX

    ; Send drive + LBA bits 24-27
    mov dx, 0x1F6
    mov al, bl
    shr eax, 24
    or al, 0xE0             ; 1110 0000b: LBA mode + master drive
    out dx, al

    ; Send number of sectors
    mov dx, 0x1F2
    mov al, cl
    out dx, al

    ; Send LBA bits 0-7
    mov dx, 0x1F3
    mov eax, ebx
    out dx, al

    ; Send LBA bits 8-15
    mov dx, 0x1F4
    mov eax, ebx
    shr eax, 8
    out dx, al

    ; Send LBA bits 16-23
    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al

    ; Send read command (0x20)
    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

.wait_ready:
    in al, dx
    test al, 8               ; DRQ bit set?
    jz .wait_ready

    ; Read sectors (256 words per sector)
    mov ecx, 256             ; 1 sector = 256 words (512 bytes)
    mov edx, 0x1F0           ; data port
    rep insw                 ; read from port to [EDI]

    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    popfd
    ret


;=============================================================================
; ATA write sectors (LBA mode, 32-bit protected mode)
;
; Input:
;   EAX = Logical Block Address (LBA)
;   CL  = Number of sectors to write (1-255)
;   ESI = Buffer address containing data
; Output:
;   None
;=============================================================================
ATA_LBA_WRITE:
    pushfd
    push eax
    push ebx
    push ecx
    push edx
    push esi

    mov ebx, eax            ; Save LBA in EBX

    ; Send drive + LBA bits 24-27
    mov dx, 0x1F6
    mov al, bl
    shr eax, 24
    or al, 0xE0             ; 1110 0000b: LBA mode + master drive
    out dx, al

    ; Send number of sectors
    mov dx, 0x1F2
    mov al, cl
    out dx, al

    ; Send LBA bits 0-7
    mov dx, 0x1F3
    mov eax, ebx
    out dx, al

    ; Send LBA bits 8-15
    mov dx, 0x1F4
    mov eax, ebx
    shr eax, 8
    out dx, al

    ; Send LBA bits 16-23
    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al

    ; Send write command (0x30)
    mov dx, 0x1F7
    mov al, 0x30
    out dx, al

.wait_ready:
    in al, dx
    test al, 8               ; DRQ bit set?
    jz .wait_ready

    ; Write sectors (256 words per sector)
    mov ecx, 256             ; 1 sector = 256 words = 512 bytes
    ; multiply by number of sectors
    mov bl, cl               ; sectors to write
    mov bh, 0
    mul bl                   ; eax = 256 * CL sectors
    mov ecx, eax             ; ECX = total words
    mov dx, 0x1F0            ; data port
    rep outsw                ; write from [ESI]

    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax
    popfd
    ret
ATAEND:
%endif