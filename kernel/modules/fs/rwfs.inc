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
; <data>
;
; directory block:
; 0x7788
; 16 char dirname
; <block numbers for files inside>
; 2 bytes that are empty just after, directories are limited to 123 files in a dir (same for root but you can do recursive directories)
; 
; Block numbers are dwords btw
; root is a directory block, located just after fs headers, It's name is "/", starting a path with / means root

RWFS_BUFFER: dd 0x100000

; type: function
; Returns the block number of the block for file at *eax
; -> eax string pointer to filepath
; <- eax block number (0 means non existant)
rwfs_resolveFileID:
    pushad

    mov esi, [RWFS_BUFFER]
    add esi, 512
    mov edi, eax

.next_path_component:
.skip_slash:
    mov al, [edi]
    cmp al, '/'
    jne .component_start
    inc edi
    jmp .skip_slash

.component_start:
    xor ecx, ecx
    mov ebx, edi
.read_component:
    mov al, [edi]
    cmp al, '/'                        ; end of component?
    je .component_done
    cmp al, 0                           ; null terminator?
    je .component_done
    cmp ecx, 16
    je .component_done                  ; max 16 chars
    mov [COMPONENT_BUFFER + ecx], al
    inc ecx
    inc edi
    jmp .read_component
.component_done:
    mov byte [COMPONENT_BUFFER + ecx], 0 ; null-terminate buffer
    ; now COMPONENT_BUFFER has the name to find

    ; search current directory block for matching name
    mov edx, esi                        ; edx = current dir block
    call rwfs_findInDir                  ; returns eax = block number (0 if not found)
    cmp eax, 0
    je .not_found                        ; component not found
    ; eax now holds the block of next dir/file
    mov esi, eax

    ; check if end of path string
    mov al, [edi]
    cmp al, 0
    je .done                             ; reached end, esi = file block
    cmp al, '/'
    je .next_path_component
    ; else unexpected char, could handle error here

.done:
    mov eax, esi                         ; return block number
    jmp .cleanup

.not_found:
    xor eax, eax                          ; return 0

.cleanup:
    popad
    ret

; type: function
; Initiates the RWFS
; Reads sectors starting from LBA in eax, stops after MAX_SECTORS or NULL sector
; -> eax Input sector LBA
rwfs_init:
    jmp .skip
    .localstr: db "[RWFS] Finished INIT", 0
.skip:
    pushad

    mov esi, 0                  ; sector counter
    mov ecx, 512h        ; maximum sectors to read
    ; eax already contains starting LBA
    lea edi, [RWFS_BUFFER]      ; edi = buffer for reading sectors

.read_next_sector:
    cmp esi, ecx
    jae .done                   ; reached max sectors, stop

    mov bl, 0                   ; 0 = read
    call ATA_DISK_SERVICE        ; read 1 sector at eax LBA into edi

    ; check if sector is all NULL bytes
    xor edx, edx                 ; edx = offset in sector
.check_sector:
    cmp byte [edi + edx], 0
    jne .sector_has_data         ; if any byte != 0, sector has data
    inc edx
    cmp edx, 512                 ; finished sector?
    jne .check_sector
    ; entire sector is NULL, stop
    jmp .done
.sector_has_data:
    inc eax                       ; next LBA
    inc esi                       ; increment sector counter
    jmp .read_next_sector

.done:
    popad
    printk .localstr, 0x0F
    ret

; type: function
; Moves a read block of data
; -> eax string pointer to filepath
; -> ebx amount of bytes to read
; -> ecx starting index
; <- eax ram buffer address
rwfs_read:
    ret

; type: function
; Writes a block of data to a file
; -> eax string pointer to filename
; -> ebx pointer to block of data
; -> ecx size of that block
rwfs_write:
    ret

; type: function
; Creates a file in rwfs
; -> eax string pointer to file path
; -> ebx string pointer to file name
; <- eax block number for file
rwfs_create:
    ret

; type: function
; Remvoes a file in rwfs
; -> eax string pointer to file path
; <- ax error code (0 = no errors)
rwfs_rm:
    ret

; type: function
; Make a directory in rwfs
; -> eax string pointer to dir path
; -> ebx string pointer to dir name
; <- eax block number for directory
rwfs_mkdir:
    ret

; type: function
; Searches for a file or directory in a given directory block
; -> edx = address of directory block
; -> COMPONENT_BUFFER = pointer to 16-byte name to search
; <- eax = block number of found file/dir (0 if not found)
rwfs_findInDir:
    pushad
    mov esi, edx            ; esi = current directory block address
.next_dir_block:
    ; directory name is first 16 bytes (skip it)
    add esi, 16

    mov ecx, 0              ; entry counter
    mov edi, esi            ; edi = pointer to first entry in this block

.check_entry:
    cmp ecx, 123            ; max entries per block
    je .next_block           ; reached end of entries in this block

    ; read entry block number
    mov eax, [edi + 16]      ; each entry: 16 bytes name + 4 bytes block number
    cmp eax, 0
    je .skip_entry           ; skip empty entry

    ; compare entry name to COMPONENT_BUFFER
    mov ebx, edi             ; pointer to entry name
    mov edx, COMPONENT_BUFFER
    mov esi, 16              ; compare 16 bytes
    xor eax, eax             ; zero = match flag

.compare_loop:
    mov al, [ebx]
    cmp al, [edx]
    jne .not_match
    inc ebx
    inc edx
    dec esi
    jnz .compare_loop
    ; matched all 16 bytes
    mov eax, [edi + 16]      ; return the block number of this entry
    jmp .found

.not_match:
.skip_entry:
    add edi, 20              ; move to next entry (16+4 bytes)
    inc ecx
    jmp .check_entry

.next_block:
    ; check if this directory has a next block
    mov eax, [esi + 512 - 4]  ; last DWORD in block = next block
    cmp eax, 0
    je .not_found             ; no more blocks, not found
    ; move to next block
    mov esi, eax
    jmp .next_dir_block

.found:
    ; eax already has the block number
    popad
    ret

.not_found:
    xor eax, eax
    popad
    ret

COMPONENT_BUFFER: times 17 db 0