; RWFS PARTITION FMT
; Headers: 1 sector
; "Blocks", each block is 1 sector
; First Block of file: 0xAA55
; First Block of fs: 0x8362
; Any Block within a file: 0x2631
; Last Block of a file: 0x7361
; First block of a directory: 0x7788
;  
; A block is just:
; block magic
; <data>
; next block number for the same block chain (file, directory, can be data if it's a end of chain block)
; any block with a broken magic number is considered unallocated, not corrupted
;
; Special thing: files use a 16 byte long name ;) it is at the beginning of the file just after the magic
; Directories are just a list of block numbers (DWORDS)
; Header of the fs also has a dword to say how many blocks are in the thing, block size is always 512

; fs header block:
; 0x8362
; dword amount of blocks in the fs
; 
; first block of file:
; 0xAA55
; 16 char non null terminated string: filename (can be less then 16)
; dword next block within this file (0 means it's the last block, it supports it)
; <data here>
;
; block within file:
; 0x2631
; block number of the first block of this file
; 
; directory block:
; 0x7788
; 16 char dirname
; <block numbers for files inside>
; 
; Block numbers are dwords btw