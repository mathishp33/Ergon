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
    .space PROC_MAX * PCB_SIZE

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

    movi r1, P_NONE ; pas de parent
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

  ; proc_alloc : r1 = pid du parent (P_NONE = aucun)
  ; -> r0 = pid ou -1 ; r1 = base, r2 = sommet (valides si r0 >= 0)
  ; clobber : r3, r4, r5, r6
  proc_alloc:
    mov r6, r1 ; parent
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
    shli r4, r3, PCB_SHIFT
    leab r0, proc_tbl
    add r4, r4, r0
    lbasew r0, r4, P_STATE
    cmpi r0, P_FREE
    jnz pa_next
    movi r0, P_RUN
    sbasew r0, r4, P_STATE
    sbasew r1, r4, P_BASE
    sbasew r2, r4, P_TOP
    sbasew r6, r4, P_PARENT
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
  ; Démarre le processus cur_pid. Ne revient pas.
  enter_user:
    movi sp, KERNEL_STACK_TOP
    setub r4 ; région mémoire autorisée en USER
    setul r2
    sub r3, r2, r3 ; stack_limit = sommet - taille de pile (celle du header)
    setsl r3
    ldw r5, cur_pid
    shli r5, r5, PCB_SHIFT
    leab r6, proc_tbl
    add r5, r5, r6
    sbasew r3, r5, P_SLIM ; pour resume_cur
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
    cmpi r0, SYS_SPAWN
    jz sys_spawn
    movi r0, SYS_ENOSYS

  trap_exit:
    pop cmp
    pop r6
    pop r5
    pop r4
    sysret

  fault:
    lbasew r4, sp, 16 ; PC et mode sauvegardé (sous cmp, r6, r5, r4)
    andi r4, r4, 1
    cmpi r4, 0
    jz kfault
    addi r1, trp, 128 ; code de sortie = 128 + cause
    jmp proc_exit

  kfault:
    addi r0, trp, 128
    halt

  sys_exit:
    andi r1, r1, 255 ; les codes négatifs sont réservés aux erreurs du kernel
    jmp proc_exit

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
    call launch ; ne revient que sur erreur
    call ld_errno
    jmp trap_exit

; ld_errno : r0 = LD_* -> r0 = erreur négative
  ld_errno:
    cmpi r0, LD_NOENT
    jz le_noent
    cmpi r0, LD_BADEXEC
    jz le_bad
    movi r0, SYS_EIO
    ret
  le_noent:
    movi r0, SYS_ENOENT
    ret
  le_bad:
    movi r0, SYS_EBADEXEC
    ret

  ; zero_region : r1 = début, r2 = fin (exclue), multiples de 4. clobber : r1, r3
  zero_region:
    clr r3
  zr_loop:
    cmpu r1, r2
    jz zr_done
    sbasew r3, r1, 0
    addi r1, r1, 4
    jmp zr_loop
  zr_done:
    ret

  ; ctx_save : sauvegarde le contexte USER du processus courant dans son PCB.
  ; À appeler UNIQUEMENT depuis un handler au niveau de pile de trap_entry :
  ; [sp]=retour  [sp+4]=cmp  [sp+8]=r6  [sp+12]=r5  [sp+16]=r4  [sp+20]=PC|mode
  ; r4, r5, r6 sont déjà sur la pile : on s'en sert comme scratch. r0 n'est pas sauvegardé.
  ctx_save:
    ldw r4, cur_pid
    shli r4, r4, PCB_SHIFT
    leab r5, proc_tbl
    add r4, r4, r5 ; r4 = PCB
    sbasew r1, r4, P_R1
    sbasew r2, r4, P_R2
    sbasew r3, r4, P_R3
    sbasew r7, r4, P_R7
    sbasew r8, r4, P_R8
    sbasew r9, r4, P_R9
    sbasew r10, r4, P_R10
    lbasew r5, sp, 4
    sbasew r5, r4, P_CMP
    lbasew r5, sp, 8
    sbasew r5, r4, P_R6
    lbasew r5, sp, 12
    sbasew r5, r4, P_R5
    lbasew r5, sp, 16
    sbasew r5, r4, P_R4
    lbasew r5, sp, 20
    sbasew r5, r4, P_PC
    getusp r5
    sbasew r5, r4, P_USP
    ret

  ; resume_cur : r0 = valeur de retour du syscall en attente du processus cur_pid.
  ; Restaure son contexte et retourne en USER. Ne revient pas.
  resume_cur:
    ldw r4, cur_pid
    shli r4, r4, PCB_SHIFT
    leab r5, proc_tbl
    add r4, r4, r5 ; r4 = PCB
    lbasew r5, r4, P_BASE
    setub r5
    lbasew r5, r4, P_TOP
    setul r5
    lbasew r5, r4, P_SLIM
    setsl r5
    lbasew r5, r4, P_USP
    setusp r5
    movi sp, KERNEL_STACK_TOP
    lbasew r5, r4, P_PC
    push r5 ; sysret le dépile : mode = USER, PC = instruction après le syscall
    lbasew r1, r4, P_R1
    lbasew r2, r4, P_R2
    lbasew r3, r4, P_R3
    lbasew r7, r4, P_R7
    lbasew r8, r4, P_R8
    lbasew r9, r4, P_R9
    lbasew r10, r4, P_R10
    lbasew cmp, r4, P_CMP
    lbasew r6, r4, P_R6
    lbasew r5, r4, P_R5
    lbasew r4, r4, P_R4 ; en dernier : r4 est le pointeur
    sysret

  ; proc_exit : r1 = code de sortie. Termine le processus courant. Ne revient pas.
  proc_exit:
    mov r7, r1 ; code
    ldw r8, cur_pid
    shli r4, r8, PCB_SHIFT
    leab r5, proc_tbl
    add r4, r4, r5 ; r4 = PCB du mourant
    lbasew r9, r4, P_PARENT
    cmpi r9, 0
    jl root_exit ; P_NONE (-1) : processus racine

    lbasew r1, r4, P_BASE ; on efface la région : rien ne fuit vers le prochain occupant
    lbasew r2, r4, P_TOP
    call zero_region ; préserve r4
    movi r5, P_FREE
    sbasew r5, r4, P_STATE

    stw r9, cur_pid ; le parent redevient le processus courant
    shli r4, r9, PCB_SHIFT
    leab r5, proc_tbl
    add r4, r4, r5 ; r4 = PCB du parent
    lbasew r5, r4, P_BASE
    stw r5, cur_base
    lbasew r5, r4, P_TOP
    stw r5, cur_top
    movi r5, P_RUN
    sbasew r5, r4, P_STATE
    mov r0, r7 ; valeur de retour de son SYS_SPAWN
    jmp resume_cur

  root_exit:
    mov r0, r7 ; EnvironmentManager::start() lit r0
    halt

  ; sys_spawn : r1 = indice du programme
  sys_spawn:
    call ctx_save ; contexte du parent sauvegardé (r1 inchangé)
    mov r7, r1 ; r7..r10 sont libres : ils sont dans le PCB
    ldw r1, cur_pid ; parent du futur fils
    call proc_alloc ; r0 = pid, r1 = base, r2 = sommet
    cmpi r0, 0
    jl spawn_noslot
    mov r8, r0 ; pid du fils
    mov r9, r1 ; base du fils
    mov r10, r2 ; sommet du fils
    mov r1, r7
    mov r2, r9
    mov r3, r10
    call exec_load ; r0 = LD_*, r1 = entrée, r2 = sommet, r3 = taille de pile
    cmpi r0, 0
    jnz spawn_failed

    ldw r5, cur_pid ; le parent passe en attente
    shli r5, r5, PCB_SHIFT
    leab r6, proc_tbl
    add r5, r5, r6
    movi r6, P_WAIT
    sbasew r6, r5, P_STATE
    stw r8, cur_pid ; le fils devient le processus courant
    stw r9, cur_base
    stw r10, cur_top
    mov r4, r9 ; r1 = entrée, r2 = sommet, r3 = taille de pile, r4 = base
    jmp enter_user

  spawn_noslot:
    movi r0, SYS_ENOMEM
    jmp resume_cur

  spawn_failed: ; le parent n'a pas été touché (le fils est dans un autre slot)
    call ld_errno ; r0 = erreur
    mov r1, r9 ; le slot a pu être écrit à moitié
    mov r2, r10
    call zero_region ; ne touche pas r0
    shli r5, r8, PCB_SHIFT
    leab r6, proc_tbl
    add r5, r5, r6
    movi r6, P_FREE
    sbasew r6, r5, P_STATE
    jmp resume_cur ; le parent reprend avec r0 < 0 et tous ses registres