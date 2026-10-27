; pt_fault : écrit dans la mémoire du kernel -> faute (cause 5) -> le kernel le tue avec 128 + 5
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  movi r1, 0x1000
  sbasew r2, r1, 0
  movi r1, 0
  movi r0, SYS_EXIT
  syscall
