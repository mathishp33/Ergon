%equ KERNEL_BASE 0x1000
%equ KERNEL_STACK_TOP 0xF000
%equ USER_BASE 0x10000
%equ BOOT_MAGIC 0x4E475245
%equ EXEC_MAGIC 0x43455845
%equ DISK_VERSION 1
%equ BLOCK_SIZE 512

; ---- SUPERBLOCK (bloc 0) ----
%equ SB_MAGIC 0
%equ SB_VERSION 4
%equ SB_BSIZE 8
%equ SB_TOTAL 12
%equ SB_KSTART 16
%equ SB_KBLOCKS 20
%equ SB_KLOAD 24
%equ SB_KENTRY 28
%equ SB_TBL_START 32
%equ SB_TBL_COUNT 36

; ---- EXEC TABLE (bloc 1), 32 oct/entrée ----
%equ ET_START 16
%equ ET_BLOCKS 20

; ---- HEADER EXEC (premier bloc du programme) ----
%equ XH_MAGIC 0
%equ XH_LOAD 4
%equ XH_ENTRY 8
%equ XH_IMAGE 12
%equ XH_BSS 16
%equ XH_STACK 20

; ---- RETURN LOADER CODES ----
%equ LD_OK 0
%equ LD_IO 1
%equ LD_BADFS 2
%equ LD_NOENT 3
%equ LD_BADEXEC 4