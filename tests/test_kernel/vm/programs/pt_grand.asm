; pt_grand : SPAWN imbriqué (pt_exit7) puis sort avec (code du petit-fils + 1) = 8
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  movi r1, 6
  movi r0, SYS_SPAWN
  syscall
  addi r1, r0, 1
  movi r0, SYS_EXIT
  syscall
