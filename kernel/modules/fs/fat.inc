%ifdef FAT32
[bits 32]
jmp ENDFAT

;-----------------------------------------------
; Constant BPB (pre-filled, can adjust as needed)
;-----------------------------------------------
BPB:
    .BS_jmpBoot:      times 3 db 0
    .BS_OEMName:      times 8 db 0
    .BPB_BytsPerSec:  dw 512       ; bytes per sector
    .BPB_SecPerClus:  db 1
    .BPB_RsvdSecCnt:  dw 1
    .BPB_NumFATs:     db 2
    .BPB_RootEntCnt:  dw 512
    .BPB_TotSec16:    dw 2880
    .BPB_Media:       db 0xF0
    .BPB_FATSz16:     dw 9
    .BPB_SecPerTrk:   dw 18
    .BPB_NumHeads:    dw 2
    .BPB_HiddSec:     dd 0
    .BPB_TotSec32:    dd 0
    ; FAT32 fields ignored for FAT16
    .BPB_FATSz32:     dd 0
    .BPB_ExtFlags:    dw 0
    .BPB_FSVer:       dw 0
    .BPB_RootClus:    dd 0
    .BPB_FSInfo:      dw 0
    .BPB_BkBootSec:   dw 0
    .BPB_Reserved:    times 12 db 0
    .BS_DrvNum:       db 0
    .BS_Reserved1:    db 0
    .BS_BootSig:      db 0
    .BS_VolID:        dd 0
    .BS_VolLab:       times 11 db 0
    .BS_FilSysType:   times 8 db 0

;-----------------------------------------------
; FAT16 Read Function
;-----------------------------------------------
ReadFAT16:
    pushad

    ; EAX = FAT entry number
    mov ecx, 2                    ; FAT16 entry size in bytes
    mul ecx                        ; EAX * 2 = byte offset in FAT
    mov ebx, eax                  ; save byte offset

    ; compute sector containing the FAT entry
    mov eax, FAT_LBA              ; FAT partition start LBA
    mov edx, [BPB.BPB_BytsPerSec]
    div edx                        ; EAX = sector offset, EDX = offset in sector
    add eax, FAT_LBA               ; absolute LBA sector

    ; read sector into buffer
    mov cl, 1
    lea edi, [FATBUF]
    mov bl, 0                     ; read function
    int 48

    ; extract FAT16 entry
    mov ax, [FATBUF + edx]

    popad
    ret

FATBUF: times 512 db 0

ENDFAT:
%endif
