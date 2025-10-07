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

; eax = FAT entry number
; returns: ax = FAT16 entry
ReadFAT16:
    pushad

    mov ecx, eax           ; save entry number
    mov eax, FAT_LBA       ; FAT start LBA
    mov edx, [BPB + BPB_BytsPerSec]
    mov ebx, 2             ; FAT16 entry size
    mul ebx                ; ecx*2
    add eax, FAT_LBA       ; absolute LBA
    mov esi, eax           ; save byte offset
    lea edi, [FATBUF]
    mov cl, 1
    mov bl, 0
    int 48                 ; BIOS read

    mov ax, [FATBUF + esi] ; extract entry

    popad
    ret



FATBUF: times 512 db 0

; eax = pointer to NTS
; ebx = value
define_string:
    push ecx
    push edx

    mov ecx, [STR_COUNT]       ; index to next free slot
    cmp ecx, 256
    jae define_fail            ; table full

    mov [STR_TABLE + ecx*4], eax   ; store string pointer
    mov [STR_TABLE + 1024 + ecx*4], ebx ; store value

    inc dword [STR_COUNT]
    pop edx
    pop ecx
    ret

define_fail:
    xor eax, eax              ; fail signal
    pop edx
    pop ecx
    ret

; eax = pointer to NTS
; returns: ebx = value
get_value:
    push ecx
    push edx
    mov ecx, 0
search_loop:
    cmp ecx, [STR_COUNT]
    jae not_found

    mov edx, [STR_TABLE + ecx*4] ; string pointer
    push eax                     ; save search pointer
    mov eax, edx
    call strcmp                  ; your NTS compare
    pop eax                      ; restore search pointer
    cmp eax, 0
    je found
    inc ecx
    jmp search_loop

not_found:
    xor ebx, ebx
    jmp done

found:
    mov ebx, [STR_TABLE + 1024 + ecx*4]

done:
    pop edx
    pop ecx
    ret

; eax = pointer to NTS to undef
undef_string:
    push ecx
    push edx

    mov ecx, 0
search_loop:
    cmp ecx, [STR_COUNT]
    jae undef_done            ; not found
    mov edx, [STR_TABLE + ecx*4]  ; get string pointer
    cmp edx, eax
    je undef_found
    inc ecx
    jmp search_loop

undef_done:
    pop edx
    pop ecx
    ret

undef_found:
    ; zero the pointer to mark free
    mov dword [STR_TABLE + ecx*4], 0
    mov dword [STR_TABLE + 1024 + ecx*4], 0

    ; optional: compact table by moving last entry here
    dec dword [STR_COUNT]
    cmp ecx, [STR_COUNT]
    jae undef_done            ; last entry, nothing to move
    ; move last entry into freed spot
    mov edx, [STR_TABLE + [STR_COUNT]*4]
    mov [STR_TABLE + ecx*4], edx
    mov edx, [STR_TABLE + 1024 + [STR_COUNT]*4]
    mov [STR_TABLE + 1024 + ecx*4], edx

    pop edx
    pop ecx
    ret

; each entry: 4 bytes pointer + 4 bytes value
STR_TABLE: times 1024 dd 0       ; string pointer
           times 1024 dd 0       ; values
STR_COUNT: dd 0                  ; number of defined entries

ENDFAT:
%endif
