bits 32
%define STACK_TOP 0x6000
%macro string 1
    db %1, 0
%endmacro