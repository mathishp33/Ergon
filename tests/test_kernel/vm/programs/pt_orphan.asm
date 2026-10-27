; pt_orphan : lance pt_yield en arrière-plan puis sort avec 5 sans l'attendre -> le petit-fils devient orphelin
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  movi r1, 8
  movi r0, SYS_SPAWNBG
  syscall
  movi r1, 5
  movi r0, SYS_EXIT
  syscall
