%ifdef FS_USE_RWFS
[bits 32]
jmp ENDRWFS



RWFS_BUFFER: times 512 db 0 ; Little bugfix, this was inteded to be DB, not DD

ENDRWFS:
%endif