; pt_exit7 : sort immédiatement avec le code 7
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  movi r1, 7
  movi r0, SYS_EXIT
  syscall
