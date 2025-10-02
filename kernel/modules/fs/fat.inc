%ifidn KERNEL_ROOTFS, FAT16
[bits 32]


%define FAT12 0
%define FAT16 1
%define FAT32 2

; FAT Constants
%define Partition_LBA_Begin 0 ; first byte of the partition
%define ENTRIES_PER_SECTOR 16
%define DIR_SIZE 32 ; in bytes
%define SECTOR_SIZE 64; in bytes
%define FAT_ENTRY_SIZE 4; in bytes
%define MAX_FILENAME_SIZE 8
%define MAX_EXTENTION_SIZE 3
; file table
%define TBL_OPEN_FILES 75 ; Max size of open file table
%define TBL_DIR_STATS  75 ; Max size of open directory table
; open file codes
%define MODE_READ 0
%define MODE_WRITE 1
%define MODE_BOTH 2
%define MODE_UNKNOWN 3 ; when first created and directories
; boolean support
%define TRUE 1
%define FALSE 0
%define SUCCESS 0

ROOT: db "/", 0
PARENT: db "..", 0
SELF: db ".", 0
PATH_DELIMITER: db "/", 0
DEBUG_FLAG: db "-d", 0
DEBUG: dd 0

; Parsing
%define PERIOD 46
%define ALPHA_NUMBERS_LOWER_BOUND 48
%define ALPHA_NUMBERS_UPPER_BOUND 57
%define ALPHA_UPPERCASE_LOWER_BOUND 65
%define ALPHA_UPPERCASE_UPPER_BOUND 90
%define ALPHA_LOWERCASE_LOWER_BOUND 97
%define ALPHA_LOWERCASE_UPPER_BOUND 122

ATTR_READ_ONLY: db 01h
ATTR_HIDDEN: db 02h
ATTR_SYSTEM: db 04h
ATTR_VOLUME_ID: db 08h
ATTR_DIRECTORY: db 10h
ATTR_ARCHIVE: db 20h
ATTR_LONG_NAME: db 0Fh

FAT_FREE_CLUSTER: dd 0
FAT_EOC: dd 0FFFFFF8h
MASK_IGNORE_MSB: dd 0FFFFFFFh

%endif