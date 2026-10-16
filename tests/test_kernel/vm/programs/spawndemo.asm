%include "syscalls.inc"

.entry main

.section .text
  main:
    movi r7, 1234 ; doit survivre aux SYS_SPAWN

    movi r0, SYS_SPAWN
    movi r1, 6 ; "child" : sort avec 42
    syscall
    cmpi r0, 42
    jnz fail1
    cmpi r7, 1234
    jnz fail2

    movi r0, SYS_SPAWN
    movi r1, 3 ; "memfault" : tué par la faute 5
    syscall
    cmpi r0, 133
    jnz fail3

    movi r0, SYS_SPAWN
    movi r1, 4 ; "stackovf" : tué par la faute 4
    syscall
    cmpi r0, 132
    jnz fail4

    movi r0, SYS_SPAWN
    movi r1, 99 ; indice inexistant
    syscall
    cmpi r0, SYS_ENOENT
    jnz fail5

    movi r8, 12 ; plus que PROC_MAX : échoue si les slots ne sont pas libérés
  again:
    movi r0, SYS_SPAWN
    movi r1, 6
    syscall
    cmpi r0, 42
    jnz fail6
    dec r8
    cmpi r8, 0
    jnz again

    movi r0, SYS_WRITE
    leab r1, msg
    movi r2, 3
    syscall
    movi r0, SYS_EXIT
    movi r1, 0
    syscall

  fail1:
    movi r1, 1
    jmp die
  fail2:
    movi r1, 2
    jmp die
  fail3:
    movi r1, 3
    jmp die
  fail4:
    movi r1, 4
    jmp die
  fail5:
    movi r1, 5
    jmp die
  fail6:
    movi r1, 6
  die:
    movi r0, SYS_EXIT
    syscall

.section .rodata
  msg:
    .asciz "OK\n"