BITS 32

;========================
; Table of keys + values
;========================
TBL_ARR: times 1024 dd 0        ; string offsets
         times 1024 dd 0        ; values
TBL_FREE: dd 0                  ; next free slot (max used)

;========================
; String heap
;========================
STRING_HEAP: times 16384 db 0   ; 16 KB for strings
STRING_HEAP_POS: dd 0            ; next free offset in STRING_HEAP

;========================
; define eax=str_ptr, ebx=value
;========================
define:
    push esi
    push edi

    mov esi, eax                ; source string pointer
    mov edi, [STRING_HEAP_POS]  ; dest offset
    mov edx, STRING_HEAP
    add edx, edi                ; destination address

.copy_loop:
    mov al, [esi]
    mov [edx], al
    inc esi
    inc edx
    cmp al, 0
    jne .copy_loop

    ; find free slot in table (reuse empty slots)
    xor ecx, ecx                ; ecx = slot index
.find_slot:
    cmp ecx, 1024
    jae .done
    mov eax, [TBL_ARR + ecx*4]
    cmp eax, 0
    je .use_slot
    inc ecx
    jmp .find_slot

.use_slot:
    mov [TBL_ARR + ecx*4], edi          ; store string offset
    mov [TBL_ARR + 4096 + ecx*4], ebx   ; store value

    ; update TBL_FREE if needed
    mov eax, [TBL_FREE]
    cmp ecx, eax
    jbe .skip_update
    inc ecx
    mov [TBL_FREE], ecx
.skip_update:

    ; advance string heap pointer
    mov eax, edx
    sub eax, STRING_HEAP
    mov [STRING_HEAP_POS], eax

.done:
    pop edi
    pop esi
    ret

;========================
; undef eax=str_ptr
;========================
undef:
    push esi
    push edi

    mov esi, eax
    xor edi, edi

.search_loop:
    cmp edi, [TBL_FREE]
    jae .done_undef

    mov ecx, [TBL_ARR + edi*4]     ; string offset
    cmp ecx, 0
    je .next_slot

    mov edx, STRING_HEAP
    add edx, ecx
    mov ebx, esi

.cmp_loop:
    mov al, [edx]
    mov bl, [ebx]
    cmp al, bl
    jne .next_slot
    cmp al, 0
    je .found_slot
    inc edx
    inc ebx
    jmp .cmp_loop

.next_slot:
    inc edi
    jmp .search_loop

.found_slot:
    mov dword [TBL_ARR + edi*4], 0
    mov dword [TBL_ARR + 4096 + edi*4], 0
.done_undef:
    pop edi
    pop esi
    ret

;========================
; get eax=str_ptr -> returns eax=value
;========================
get:
    push esi
    push edi

    mov esi, eax
    xor edi, edi

.search_loop_get:
    cmp edi, [TBL_FREE]
    jae .not_found

    mov ecx, [TBL_ARR + edi*4]
    cmp ecx, 0
    je .next_entry_get

    mov edx, STRING_HEAP
    add edx, ecx
    mov ebx, esi

.cmp_loop_get:
    mov al, [edx]
    mov bl, [ebx]
    cmp al, bl
    jne .next_entry_get
    cmp al, 0
    je .found_get
    inc edx
    inc ebx
    jmp .cmp_loop_get

.next_entry_get:
    inc edi
    jmp .search_loop_get

.not_found:
    xor eax, eax
    pop edi
    pop esi
    ret

.found_get:
    mov eax, [TBL_ARR + 4096 + edi*4]
    pop edi
    pop esi
    ret

;========================
; is_def eax=str_ptr -> returns ax=0xFFFF if defined, 0 if not
;========================
is_def:
    push esi
    push edi

    mov esi, eax
    xor edi, edi

.search_loop_def:
    cmp edi, [TBL_FREE]
    jae .not_found_def

    mov ecx, [TBL_ARR + edi*4]
    cmp ecx, 0
    je .next_entry_def

    mov edx, STRING_HEAP
    add edx, ecx
    mov ebx, esi

.cmp_loop_def:
    mov al, [edx]
    mov bl, [ebx]
    cmp al, bl
    jne .next_entry_def
    cmp al, 0
    je .found_def
    inc edx
    inc ebx
    jmp .cmp_loop_def

.next_entry_def:
    inc edi
    jmp .search_loop_def

.not_found_def:
    xor ax, ax
    pop edi
    pop esi
    ret

.found_def:
    mov ax, 0FFFFh
    pop edi
    pop esi
    ret

;========================
; Macros
;========================

%macro DEFINE 2
    jmp %%.tmp
%%.str: db %1, 0
%%.tmp:
    mov eax, %%.str
    mov ebx, %2
    call define
%endmacro

%macro UNDEF 1
    jmp %%.tmp
%%.str: db %1, 0
%%.tmp:
    mov eax, %%.str
    call undef
%endmacro

%macro GET 2
    jmp %%.tmp
%%.str: db %1, 0
%%.tmp:
    mov eax, %%.str
    call get
    mov %2, eax
%endmacro

%macro IS_DEF 2
    jmp %%.tmp
%%.str: db %1, 0
%%.tmp:
    mov eax, %%.str
    call is_def
    mov %2, ax
%endmacro
