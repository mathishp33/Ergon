%include "mmio.inc"

.global disk_read
.global disk_write
.global disk_block_count

.section .text
  ; clobber: r3, r4, r5
  disk_read: ; r1 = bloc, r2 = adresse RAM -> r0 = statut (0 = OK)
    movi r3, DISK_CMD_READ
    jmp disk_xfer

  disk_write: ; r1 = bloc, r2 = adresse disque
    movi r3, DISK_CMD_WRITE

  disk_xfer: ; r1 = bloc, r2 = adresse, r3 = commande
    movi r4, MMIO_BASE
    addi r5, r4, DISK_BLOCK
    sbasew r1, r5, 0
    addi r5, r4, DISK_BUFFER
    sbasew r2, r5, 0
    addi r5, r4, DISK_CMD
    sbasew r3, r5, 0 ; synchrone dans la VM
    addi r5, r4, DISK_STATUS
    lbasew r0, r5, 0
    ret

  ; disk_block_count : -> r0
  disk_block_count:
    movi r4, MMIO_BASE
    addi r4, r4, DISK_BLOCK_COUNT
    lbasew r0, r4, 0
    ret