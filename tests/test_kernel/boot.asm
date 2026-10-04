%equ MMIO_HI 0xF000
%equ DISK_BLOCK 0x0100
%equ DISK_BUFFER 0x0104
%equ DISK_CMD 0x0108
%equ DISK_STATUS 0x010C
%equ HDR_BUF 0x0400
%equ BLOCK_SIZE 512
%equ BOOT_MAGIC 0x4E475245

.section .text
  .entry boot

  ; disk_read_block : r1 = n° de bloc, r2 = adresse RAM ; retourne r0 = status (0 = OK)
  ; clobber : r5, r6 ; suppose r10 = MMIO_BASE
    disk_read_block:
      addi r5, r10, DISK_BLOCK
      sbasew r1, r5, 0
      addi r5, r10, DISK_BUFFER
      sbasew r2, r5, 0
      movi r6, 1 ; 1 = lecture (disque -> RAM)
      addi r5, r10, DISK_CMD
      sbasew r6, r5, 0 ; synchrone dans la VM
      addi r5, r10, DISK_STATUS
      lbasew r0, r5, 0
      ret

  boot:
      movi r10, MMIO_HI
      shli r10, r10, 16 ; r10 = MMIO_BASE

      ; header
      movi r1, 0
      movi r2, HDR_BUF
      call disk_read_block
      cmpi r0, 0
      jnz boot_fail

      movi r3, HDR_BUF
      lbasew r4, r3, 0 ; magic
      cmpi r4, BOOT_MAGIC
      jnz boot_fail
      lbasew r7, r3, 4 ; load_addr
      lbasew r8, r3, 8 ; entry
      lbasew r9, r3, 12 ; n_blocks

      ; boucle de chargement : blocs_1..blocks_n -> load_addr
      movi r1, 1 ; bloc courant
      mov r2, r7 ; adresse RAM courante
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
    push r8 ; saut indirect: empile
    ret ; dépile dans PC

  boot_fail:
      movi r0, 255
      halt