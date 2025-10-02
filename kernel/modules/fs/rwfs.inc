%ifdef FS_USE_RWFS
[bits 32]
jmp ENDRWFS



RWFS_BUFFER: times 512 dd 0

ENDRWFS:
%endif