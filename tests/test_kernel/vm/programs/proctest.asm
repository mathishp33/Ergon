; proctest : batterie de tests des processus (SPAWN, SPAWNBG, WAIT, YIELD, GETPID, EXIT, fautes).
; Doit être lancé comme processus racine (BOOT_PROG = indice de proctest).
; Sortie : '.' par vérification réussie, 'X' par échec ; 'a','b','c' = marqueurs de la racine,
; 'y' = marqueur de pt_yield. Code de sortie = nombre d'échecs (0 = tout est bon).
%include "syscalls.inc"
.entry main
.stack_size 0x800

; indices dans la table de programmes (ordre de kernel_test.cpp : child est l'indice 4)
%equ PT_EXIT7 6
%equ PT_FAULT 7
%equ PT_YIELD 8
%equ PT_REGS 9
%equ PT_GRAND 10
%equ PT_ORPHAN 11

; sys(n) : appel système n
%macro sys(n)
  movi r0, n
  syscall
%endmacro

; putc(ch) : écrit un caractère (clobber r0, r1, r2, r4)
%macro putc(ch)
  movi r4, ch
  push r4
  mov r1, sp
  movi r2, 1
  sys(SYS_WRITE)
  pop r4
%endmacro

; emit : écrit '.' si r5 == 0, 'X' sinon ; r10 compte les échecs (clobber r0, r1, r2, r4, r5)
%macro emit()
  add r10, r10, r5
  muli r5, r5, 42
  addi r4, r5, 46
  push r4
  mov r1, sp
  movi r2, 1
  sys(SYS_WRITE)
  pop r4
%endmacro

; checki(reg, val) : vérifie reg == val
%macro checki(reg, val)
  subi r5, reg, val
  abs r5, r5
  mini r5, r5, 1
  emit()
%endmacro

; checkr(a, b) : vérifie a == b
%macro checkr(a, b)
  sub r5, a, b
  abs r5, r5
  mini r5, r5, 1
  emit()
%endmacro

.section .text
main:
  clr r10 ; compteur d'échecs

  ; T0 : pid de la racine
  sys(SYS_GETPID)
  checki(r0, 0)

  ; T1-T4 : SPAWN synchrone -> code de sortie du fils
  movi r1, PT_EXIT7
  sys(SYS_SPAWN)
  checki(r0, 7)
  movi r1, 99 ; indice inexistant
  sys(SYS_SPAWN)
  checki(r0, SYS_ENOENT)
  movi r1, PT_FAULT ; le fils fait une faute mémoire : 128 + 5
  sys(SYS_SPAWN)
  checki(r0, 133)
  movi r1, PT_GRAND ; SPAWN imbriqué : le petit-fils sort avec 7, le fils avec 7 + 1
  sys(SYS_SPAWN)
  checki(r0, 8)

  ; T5 : le contexte du parent survit à un SPAWN, et le fils démarre avec des registres à zéro
  movi fp, 0x1234
  movi r7, 0x7777
  movi r8, 0x8888
  movi r9, 0x9999
  movi r1, 0x1111
  fmov f1, r1
  movi r1, PT_REGS
  sys(SYS_SPAWN)
  checki(r0, 0) ; 0 = r0..r10 et fp valaient 0 au démarrage du fils
  checki(fp, 0x1234)
  movf r6, f1
  checki(r6, 0x1111)
  checki(r7, 0x7777)
  checki(r8, 0x8888)
  checki(r9, 0x9999)

  ; T6 : SPAWNBG puis WAIT sur ce pid précis
  movi r1, PT_EXIT7
  sys(SYS_SPAWNBG)
  mov r6, r0
  checki(r0, 1) ; le plus petit slot libre
  mov r1, r6
  sys(SYS_WAIT)
  mov r7, r1
  checki(r0, 7)
  checkr(r7, r6)

  ; T7 : WAIT(-1) avec deux fils ; le second est devenu zombie pendant qu'on était réveillé
  movi r1, PT_EXIT7
  sys(SYS_SPAWNBG) ; pid 1
  movi r1, PT_GRAND
  sys(SYS_SPAWNBG) ; pid 2
  movi r1, -1
  sys(SYS_WAIT)
  mov r7, r1
  checki(r0, 7)
  checki(r7, 1)
  movi r1, -1
  sys(SYS_WAIT)
  mov r7, r1
  checki(r0, 8)
  checki(r7, 2)

  ; T8 : plus aucun fils
  movi r1, -1
  sys(SYS_WAIT)
  checki(r0, SYS_ECHILD)

  ; T9 : YIELD, entrelacement avec un fils en arrière-plan (attendu : a y b y c y)
  movi r1, PT_YIELD
  sys(SYS_SPAWNBG)
  mov r6, r0
  putc(97)
  sys(SYS_YIELD)
  putc(98)
  sys(SYS_YIELD)
  putc(99)
  sys(SYS_YIELD)
  mov r1, r6
  sys(SYS_WAIT)
  checki(r0, 2)

  ; T10 : orphelin. Le fils sort avec 5 sans attendre son propre fils (qui yield 3 fois).
  movi r1, PT_ORPHAN
  sys(SYS_SPAWN)
  checki(r0, 5)
  sys(SYS_YIELD)
  sys(SYS_YIELD)
  sys(SYS_YIELD)
  sys(SYS_YIELD)

  ; T11 : aucun slot perdu (orphelin libéré), puis table pleine, puis ramassage de tous les fils
  movi r6, 7 ; PROC_MAX - 1 fils possibles
  movi r7, 1 ; pid attendu
fill:
  movi r1, PT_EXIT7
  sys(SYS_SPAWNBG)
  checkr(r0, r7)
  inc r7
  dec r6
  cmpi r6, 0
  jnz fill
  movi r1, PT_EXIT7
  sys(SYS_SPAWNBG)
  checki(r0, SYS_ENOMEM)
reap:
  movi r1, -1
  sys(SYS_WAIT)
  cmpi r0, 0
  jl reaped
  jmp reap
reaped:
  checki(r0, SYS_ECHILD)
  mov r1, r10
  sys(SYS_EXIT)
