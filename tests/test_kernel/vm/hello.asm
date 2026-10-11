%include "syscalls.inc"

.entry main

.section .text
  main:
    movi r0, SYS_WRITE
    leab r1, prompt
    movi r2, 2
    syscall

    movi r0, SYS_READ
    leab r1, buf
    movi r2, 64
    syscall

    mov r2, r0 ; nombre d'octets lus
    movi r0, SYS_WRITE
    leab r1, buf
    syscall

    movi r0, SYS_EXIT
    movi r1, 0
    syscall

.section .rodata
  prompt:
   .asciz "> "

.section .bss
  buf:
   .space 64