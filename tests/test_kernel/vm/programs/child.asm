%include "syscalls.inc"

.entry main

.section .text
  main:
    movi r0, SYS_EXIT
    movi r1, 42
    syscall