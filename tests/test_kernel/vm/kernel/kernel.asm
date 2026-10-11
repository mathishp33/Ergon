%include "layout.inc"
%include "mmio.inc"
%include "syscalls.inc"

%equ BOOT_PROG 0

.entry kmain
.extern loader_init
.extern exec_load

.section .bss
  k_user_top:
    .space 4
  cur_pid:
    .space 4
  cur_base:
    .space 4
  cur_top:
    .space 4
  proc_tbl:
    .space PROC_MAX * 16

.section .text
  kmain:
    mov r7, sp ; boot a laissé SP au sommet de la RAM
    stw r7, k_user_top
    movi sp, KERNEL_STACK_TOP ; pile kernel
    setksp sp
    leab r1, trap_entry
    settv r1

    call loader_init
    cmpi r0, 0
    jnz panic

    call proc_alloc ; r0 = pid, r1 = base, r2 = sommet
    cmpi r0, 0
    jl panic_noslot
    stw r0, cur_pid
    stw r1, cur_base
    stw r2, cur_top

    movi r1, BOOT_PROG
    call launch ; ne revient que sur erreur (r0 = LD_*)

  panic_noslot:
    movi r0, 110
    halt

  ; proc_alloc : -> r0 = pid ou -1 ; r1 = base, r2 = sommet (valides si r0 >= 0)
  ; clobber : r3, r4, r5
  proc_alloc:
    clr r3
    ldw r5, k_user_top

  pa_loop:
    cmpi r3, PROC_MAX
    jz pa_none
    shli r4, r3, PROC_SLOT_SHIFT
    addi r1, r4, PROC_SLOT_BASE ; base du slot
    addi r2, r1, PROC_SLOT_SIZE ; sommet du slot
    cmpu r2, r5
    jg pa_none ; ce slot et les suivants dépassent la RAM
    shli r4, r3, 4
    leab r0, proc_tbl
    add r4, r4, r0
    lbasew r0, r4, P_STATE
    cmpi r0, 0
    jnz pa_next
    movi r0, 1
    sbasew r0, r4, P_STATE
    sbasew r1, r4, P_BASE
    sbasew r2, r4, P_TOP
    mov r0, r3
    ret

  pa_next:
    inc r3
    jmp pa_loop

  pa_none:
    movi r0, -1
    ret

  ; launch : r1 = indice de la table
  ; continue si OK, sinon r0 = LD_*
  launch:
    ldw r2, cur_base
    ldw r3, cur_top
    call exec_load ; r1 = entrée, r2 = sommet, r3 = taille de pile
    cmpi r0, 0
    jnz launch_fail
    ldw r4, cur_base
    jmp enter_user

  launch_fail:
    ret

  ; enter_user : r1 = entrée, r2 = sommet de région, r3 = taille de pile, r4 = base de région
  ; Ne revient pas. Repart d'une pile kernel vide (un trap USER recharge SP depuis kernel_sp).
  enter_user:
    movi sp, KERNEL_STACK_TOP
    setub r4 ; région mémoire autorisée en USER
    setul r2
    sub r3, r2, r3 ; stack_limit = sommet - taille de pile (celle du header)
    setsl r3
    setusp r2
    ori r1, r1, 1 ; bit 0 = retour en USER
    push r1
    sysret

  panic: ; code de sortie = 100 + statut
    addi r0, r0, 100
    halt

  trap_entry:
    push r4
    push r5
    push r6
    push cmp ; flags de l'utilisateur
    cmpi trp, 0 ; 0 = syscall, 1..3 = faute
    jnz fault
    cmpi r0, SYS_EXIT
    jz sys_exit
    cmpi r0, SYS_WRITE
    jz sys_write
    cmpi r0, SYS_READ
    jz sys_read
    cmpi r0, SYS_EXEC
    jz sys_exec
    movi r0, SYS_ENOSYS

  trap_exit:
    pop cmp
    pop r6
    pop r5
    pop r4
    sysret

  fault:
    addi r0, trp, 128 ; code de sortie = 128 + cause
    halt

  sys_exit:
    mov r0, r1
    halt

  ; check_user_buf : r1 = ptr, r2 = len -> r0 = 0 si [ptr, ptr+len) est dans la zone user
  ; clobber : r4, r5
  check_user_buf:
    ldw r4, cur_base
    cmpu r1, r4
    jl cub_bad
    add r4, r1, r2
    cmpu r4, r1 ; débordement 32 bits
    jl cub_bad
    ldw r5, cur_top
    cmpu r4, r5
    jg cub_bad
    clr r0
    ret
  cub_bad:
    movi r0, 1
    ret

  sys_write:
    call check_user_buf
    cmpi r0, 0
    jnz sys_efault
    movi r4, MMIO_BASE
    clr r5
  write_loop:
    cmpu r5, r2
    jz write_done
    lregb r6, r1, r5
    sbaseb r6, r4, CONSOLE_OUT
    inc r5
    jmp write_loop
  write_done:
    mov r0, r5
    jmp trap_exit

  sys_read:
    call check_user_buf
    cmpi r0, 0
    jnz sys_efault
    movi r4, MMIO_BASE
    clr r5 ; i = 0

  sread_loop:
    cmpu r5, r2
    jz sread_done ; buffer plein
    lbaseb r6, r4, CONSOLE_IN ; bloquant côté hôte
    cmpi r6, 0
    jz sread_done ; EOF
    sregb r6, r1, r5 ; user[ptr + i] = octet
    inc r5
    cmpi r6, 10
    jnz sread_loop ; '\n' termine la lecture (il est conservé)

  sread_done:
    mov r0, r5
    jmp trap_exit

  sys_efault:
    movi r0, SYS_EFAULT
    jmp trap_exit

  sys_exec:
    call launch ; r1 = indice (inchangé par trap_entry) ; ne revient que sur erreur
    cmpi r0, LD_NOENT
    jz exec_enoent
    cmpi r0, LD_BADEXEC
    jz exec_ebad
    movi r0, SYS_EIO
    jmp trap_exit

  exec_enoent:
    movi r0, SYS_ENOENT
    jmp trap_exit

  exec_ebad:
    movi r0, SYS_EBADEXEC
    jmp trap_exit