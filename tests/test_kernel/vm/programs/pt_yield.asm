; pt_yield : écrit 'y' puis cède la main, trois fois ; sort avec le code 2
%include "syscalls.inc"
.entry main
.stack_size 0x400
.section .text
main:
  movi r6, 3
yloop:
  movi r4, 121
  push r4
  mov r1, sp
  movi r2, 1
  movi r0, SYS_WRITE
  syscall
  pop r4
  movi r0, SYS_YIELD
  syscall
  dec r6
  cmpi r6, 0
  jnz yloop
  movi r1, 2
  movi r0, SYS_EXIT
  syscall
