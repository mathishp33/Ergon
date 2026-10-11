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
  rl_buf:
    .space 512
  tbl_start:
    .space 4
  tbl_count:
    .space 4
  ld_base:
    .space 4
  ld_top:
    .space 4
  ld_delta:
    .space 4

.section .text
  ; loader_init : lit le superblock -> r0 = LD_*
  loader_init:
    push r7
    movi r1, 0
    leab r2, sb_buf
    call disk_read
    cmpi r0, 0
    jnz mount_io
    leab r7, sb_buf
    lbasew r1, r7, SB_MAGIC
    cmpi r1, BOOT_MAGIC
    jnz mount_bad
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

  ; exec_load : r1 = indice dans la table, r2 = base de la région, r3 = sommet de la région
  ;   -> r0 = LD_*
  ;      r1 = entrée réelle, r2 = sommet de pile user, r3 = taille de pile (valides si r0 == 0)
  exec_load:
    push r7
    push r8
    push r9
    push r10
    mov r7, r1 ; indice
    stw r2, ld_base
    stw r3, ld_top
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
    shli r4, r7, 5 ; indice * 32
    leab r5, tbl_buf
    add r5, r5, r4
    lbasew r8, r5, ET_START ; r8 = bloc du header
    mov r1, r8
    leab r2, xh_buf
    call disk_read
    cmpi r0, 0
    jnz exec_io

    leab r9, xh_buf ; r9 = header (conservé jusqu'à la fin)
    lbasew r1, r9, XH_MAGIC
    cmpi r1, EXEC_MAGIC
    jnz exec_bad

    ; ---- validation des tailles par rapport à la région
    ldw r5, ld_top
    ldw r6, ld_base
    cmpu r5, r6
    jg region_ok
    jmp exec_bad

  region_ok:
    sub r5, r5, r6 ; r5 = taille de la région
    lbasew r2, r9, XH_IMAGE
    lbasew r3, r9, XH_BSS
    lbasew r4, r9, XH_STACK
    BAD_IF_ABOVE(r2, r5)
    BAD_IF_ABOVE(r3, r5)
    BAD_IF_ABOVE(r4, r5)
    addi r2, r2, 511
    shri r2, r2, 9 ; blocs d'image
    mov r10, r2
    shli r2, r2, 9
    add r2, r2, r3
    add r2, r2, r4 ; image arrondie + bss + pile (3 termes <= taille de région : pas de débordement)
    BAD_IF_ABOVE(r2, r5)

    ; au plus une relocalisation par instruction
    lbasew r2, r9, XH_NRELOC
    lbasew r3, r9, XH_IMAGE
    shri r3, r3, 3
    BAD_IF_ABOVE(r2, r3)

    ; entrée : dans l'image, alignée sur une instruction
    lbasew r1, r9, XH_ENTRY
    lbasew r3, r9, XH_LOAD ; adresse de liaison
    cmpu r1, r3
    jl exec_bad
    sub r4, r1, r3 ; offset de l'entrée
    lbasew r2, r9, XH_IMAGE
    cmpu r4, r2
    jl entry_in
    jmp exec_bad

  entry_in:
    testi r4, 7
    jnz exec_bad
    ldw r5, ld_base
    sub r5, r5, r3 ; delta = base réelle - base de liaison (modulo 2^32)
    stw r5, ld_delta

    ; ---- copie de l'image : blocs r8+1.. -> ld_base (r10 blocs)
    addi r8, r8, 1
    ldw r7, ld_base

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

  read_done: ; r8 = premier bloc de la table de relocalisation
    lbasew r10, r9, XH_NRELOC ; entrées restantes

  reloc_block:
    cmpi r10, 0
    jz reloc_done
    mov r1, r8
    leab r2, rl_buf
    call disk_read
    cmpi r0, 0
    jnz exec_io
    inc r8
    mini r3, r10, 128 ; entrées dans ce bloc
    sub r10, r10, r3
    leab r7, rl_buf
    ldw r1, ld_base
    ldw r2, ld_delta
    lbasew r5, r9, XH_IMAGE

  reloc_entry:
    cmpi r3, 0
    jz reloc_block
    lbasew r4, r7, 0 ; offset de l'imm dans l'image
    cmpu r4, r5
    jg exec_bad
    addi r6, r4, 4
    cmpu r6, r5
    jg exec_bad
    andi r6, r4, 7
    cmpi r6, 4 ; un imm est toujours à +4 d'une instruction
    jnz exec_bad
    add r6, r1, r4 ; adresse réelle de l'imm
    lbasew r4, r6, 0
    add r4, r4, r2
    sbasew r4, r6, 0
    addi r7, r7, 4
    dec r3
    jmp reloc_entry

  reloc_done:
    ; bss : zéro à partir de base + image_size (par mots)
    ldw r1, ld_base
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
    ldw r2, ld_delta
    add r1, r1, r2 ; entrée réelle
    ldw r2, ld_top
    lbasew r3, r9, XH_STACK
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