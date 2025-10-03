; =====================================
; Copyright (C) 2025 Speedruneer/SpeedruneerOff
; All rights reserved
; =====================================

; This file is fine i think

[BITS 32]
%define VIDEO_MEMORY 0xb8000
%define SCREEN_WIDTH 80
%define SCREEN_HEIGHT 25

%define BLACK          0
%define BLUE           1
%define GREEN          2
%define CYAN           3
%define RED            4
%define PURPLE         5
%define BROWN          6
%define GRAY           7
%define DARK_GRAY      8
%define LIGHT_BLUE     9
%define LIGHT_GREEN    10
%define LIGHT_CYAN     11
%define LIGHT_RED      12
%define LIGHT_PURPLE   13
%define YELLOW         14
%define WHITE          15
%define FOREGROUND     1
%define BACKGROUND     10h
; =====================================
; TXT Mode / screen routines
; =====================================

; -------------------------------------
; setCharScreen: write a single char+color at AL=col, AH=row, BL=char, BH=color
; -------------------------------------
setCharScreen:
    pushad
    mov edi, VIDEO_MEMORY
    movzx ecx, ah
    imul ecx, SCREEN_WIDTH
    movzx eax, al
    add ecx, eax
    shl ecx, 1
    add edi, ecx
    mov ax, 0
    mov al, bl
    mov ah, bh
    mov [edi], ax
    popad
    ret

; -------------------------------------
; setChar macro
; -------------------------------------
%macro setChar 4
    pushad
    mov al, %2      ; x
    mov ah, %1      ; y
    mov bl, %3      ; char
    mov bh, %4      ; color
    call setCharScreen
    popad
%endmacro

; -------------------------------------
; clearScreen macro
; -------------------------------------
%macro clearScreen 1
    pushad
    mov edi, VIDEO_MEMORY
    mov ecx, 2000
    mov al, ' '
    mov ah, %1
    movzx eax, ax

%%.clear_loop:
    mov [edi], ax
    add edi, 2
    loop %%.clear_loop
    popad
%endmacro

; -------------------------------------
; printString macro (uses ESI)
; %1 = column (x)
; %2 = row (y)
; %3 = pointer to null-terminated string
; %4 = color
; -------------------------------------
print:
    pushad
.next_char:
    lodsb
    cmp al, 0
    je .done
    mov bl, al
    mov al, cl
    call setCharScreen
    inc cl
    jmp .next_char
.done:
    popad
    ret

%macro printString 4
    pushad
    mov cl, %1
    mov ah, %2
    mov esi, %3           ; string pointer in ESI

%%next_char:
    lodsb                  ; AL = [ESI], ESI++
    cmp al, 0
    je %%done
    mov bl, al             ; character
    mov al, cl             ; column
    mov bh, %4             ; color
    call setCharScreen
    inc cl
    jmp %%next_char

%%done:
    popad
%endmacro

nibble_to_ascii:
    cmp al, 9
    jbe .digit
    add al, 'A' - 10
    ret
.digit:
    add al, '0'
    ret

int_to_char:
    push eax

    mov ah, al          ; save original AL
    shr al, 4           ; high nibble
    call nibble_to_ascii
    mov [output_of_int_to_char], al  ; store high nibble

    mov al, ah          ; restore original byte
    and al, 0x0F        ; low nibble
    call nibble_to_ascii
    mov [output_of_int_to_char+1], al ; store low nibble

    pop eax
    ret

; 2-byte buffer directly after function
output_of_int_to_char:
    dw 0    ; 2 bytes reserved
    db 0