; pt_regs : vérifie qu'il démarre avec r0..r10 et fp à zéro (code 0 = propre, 1 = fuite),
; puis écrase les registres que le parent avait posés (le parent doit les retrouver intacts).
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  or r0, r0, r1
  or r0, r0, r2
  or r0, r0, r3
  or r0, r0, r4
  or r0, r0, r5
  or r0, r0, r6
  or r0, r0, r7
  or r0, r0, r8
  or r0, r0, r9
  or r0, r0, r10
  or r0, r0, fp
  movi r1, 0
  cmpi r0, 0
  jz clean
  movi r1, 1
clean:
  movi fp, 0x5555
  movi r7, 0xAAAA
  movi r8, 0xBBBB
  movi r9, 0xCCCC
  movi r10, 0xDDDD
  movi r2, 0x2222
  fmov f1, r2
  movi r0, SYS_EXIT
  syscall
