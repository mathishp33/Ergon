%include "layout.inc"

.global loader_init
.global exec_load
.extern disk_read

; saute vers exec_bad si x > y (non signé)
%macro BAD_IF_ABOVE(x, y)
  cmpu x, y
  jg exec_bad
%endmacro

.section .bss
  sb_buf:
    .space 512
  xh_buf:
    .space 512
  tbl_buf:
    .space 512
  tbl_start:
    .space 4
  tbl_count:
    .space 4
  user_top:
    .space 4

.section .text
  ; loader_init: r1 = sommet de la RAM user -> r0 = LD_*
  loader_init:
    stw r1, user_top
    ; continues in fs_mount

  fs_mount:
    push r7
    movi r1, 0
    leab r2, sb_buf
    call disk_read
    cmpi r0, 0
    jnz mount_io
    leab r7, sb_buf
    lbasew r1, r7, SB_MAGIC
    cmpi r1, BOOT_MAGIC
    jnz  mount_bad
    lbasew r1, r7, SB_VERSION
    cmpi r1, DISK_VERSION
    jnz mount_bad
    lbasew r1, r7, SB_TBL_START
    stw r1, tbl_start
    lbasew r1, r7, SB_TBL_COUNT
    stw r1, tbl_count
    movi r0, LD_OK
    jmp mount_end

  mount_io:
    movi r0, LD_IO
    jmp mount_end

  mount_bad:
    movi r0, LD_BADFS

  mount_end:
    pop r7
    ret

  ; exec_load : r1 = indice dans la table
  ; -> r0 = LD_*, r1 = entrée, r2 = sommet de pile user (valides si r0 == 0)
  exec_load:
    push r7
    push r8
    push r9
    push r10
    mov r7, r1 ; indice
    ldw r1, tbl_count
    cmpu r7, r1
    jl exec_idx_ok
    movi r0, LD_NOENT
    jmp exec_end

  exec_idx_ok:
    ldw r1, tbl_start
    leab r2, tbl_buf
    call disk_read
    cmpi r0, 0
    jnz exec_io
    shli r4, r7, 5 ; indice * 32 (taille d'une entrée)
    leab r5, tbl_buf
    add r5, r5, r4
    lbasew r8, r5, ET_START ; r8 = bloc du header
    mov r1, r8
    leab r2, xh_buf
    call disk_read
    cmpi r0, 0
    jnz exec_io

    leab r9, xh_buf
    lbasew r1, r9, XH_MAGIC
    cmpi r1, EXEC_MAGIC
    jnz exec_bad

    ; validation : chaque terme <= sommet RAM, donc la somme tient sur 32 bits
    ldw r5, user_top
    lbasew r2, r9, XH_LOAD
    lbasew r3, r9, XH_IMAGE
    lbasew r4, r9, XH_BSS
    lbasew r6, r9, XH_STACK
    cmpui r2, USER_BASE
    jl exec_bad
    BAD_IF_ABOVE(r2, r5)
    BAD_IF_ABOVE(r3, r5)
    BAD_IF_ABOVE(r4, r5)
    BAD_IF_ABOVE(r6, r5)
    addi r3, r3, 511
    shri r3, r3, 9 ; r3 = nombre de blocs d'image
    mov r10, r3
    shli r3, r3, 9 ; octets arrondis
    add r3, r3, r2
    add r3, r3, r4
    add r3, r3, r6 ; fin = load + image + bss + pile
    BAD_IF_ABOVE(r3, r5)
    lbasew r1, r9, XH_ENTRY
    cmpu r1, r2
    jl exec_bad
    BAD_IF_ABOVE(r1, r3)

    ; lecture de l'image : blocs r8+1.. -> load_addr (r10 blocs)
    addi r8, r8, 1
    lbasew r7, r9, XH_LOAD

  read_loop:
    cmpi r10, 0
    jz read_done
    mov r1, r8
    mov r2, r7
    call disk_read
    cmpi r0, 0
    jnz exec_io
    inc r8
    addi r7, r7, BLOCK_SIZE
    dec r10
    jmp read_loop

  read_done:
    ; bss : zéro à partir de load + image_size (par mots)
    lbasew r1, r9, XH_LOAD
    lbasew r2, r9, XH_IMAGE
    add r1, r1, r2
    lbasew r2, r9, XH_BSS
    addi r2, r2, 3
    shri r2, r2, 2
    clr r3

  bss_loop:
    cmpi r2, 0
    jz bss_done
    sbasew r3, r1, 0
    addi r1, r1, 4
    dec r2
    jmp bss_loop

  bss_done:
    lbasew r1, r9, XH_ENTRY
    ldw r2, user_top
    movi r0, LD_OK
    jmp exec_end

  exec_bad:
    movi r0, LD_BADEXEC
    jmp exec_end

  exec_io:
    movi r0, LD_IO

  exec_end:
    pop r10
    pop r9
    pop r8
    pop r7
    ret