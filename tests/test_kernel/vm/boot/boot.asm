%include "layout.inc"
%include "mmio.inc"
%equ HDR_BUF 0x0400

.entry boot

.section .text
  ; disk_read_block : r1 = bloc, r2 = adresse RAM -> r0 = statut
  ; clobber : r5, r6 ; suppose r10 = MMIO_BASE
  disk_read_block:
    addi r5, r10, DISK_BLOCK
    sbasew r1, r5, 0
    addi r5, r10, DISK_BUFFER
    sbasew r2, r5, 0
    movi r6, DISK_CMD_READ
    addi r5, r10, DISK_CMD
    sbasew r6, r5, 0
    addi r5, r10, DISK_STATUS
    lbasew r0, r5, 0
    ret

  boot:
    movi r10, MMIO_BASE

    movi r1, 0
    movi r2, HDR_BUF
    call disk_read_block
    cmpi r0, 0
    jnz boot_fail

    movi r3, HDR_BUF
    lbasew r4, r3, SB_MAGIC
    cmpi r4, BOOT_MAGIC
    jnz boot_fail
    lbasew r4, r3, SB_VERSION
    cmpi r4, DISK_VERSION
    jnz boot_fail
    lbasew r1, r3, SB_KSTART ; bloc courant
    lbasew r9, r3, SB_KBLOCKS
    lbasew r2, r3, SB_KLOAD ; adresse RAM courante
    lbasew r8, r3, SB_KENTRY ; entrée du kernel
    mov r3, r9 ; blocs restants

  load_loop:
    cmpi r3, 0
    jz load_done
    call disk_read_block
    cmpi r0, 0
    jnz boot_fail
    inc r1
    addi r2, r2, BLOCK_SIZE
    dec r3
    jmp load_loop

  load_done:
    push r8
    ret

  boot_fail:
    movi r0, 255
    halt