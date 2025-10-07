bits 32

; type: function
; load a function into the global function table
; -> eax function pointer
; -> ebx function id
load_function:
    ret