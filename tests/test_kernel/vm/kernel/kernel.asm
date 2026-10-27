%include "layout.inc"
%include "mmio.inc"
%include "syscalls.inc"

%equ BOOT_PROG 5

.entry kmain
.extern loader_init
.extern exec_load

; PCB_OF(dst, pid, tmp) : dst = &proc_tbl[pid] (clobber tmp)
%macro PCB_OF(dst, pid, tmp)
  shli dst, pid, PCB_SHIFT
  leab tmp, proc_tbl
  add dst, dst, tmp
%endmacro

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
  fpu_tbl: ; 16 registres flottants par processus
    .space PROC_MAX * 64
  ; zone de passage : fstw/fldw n'adressent que des symboles, 16 mots contigus
  fpu_s0:
    .space 4
  fpu_s1:
    .space 4
  fpu_s2:
    .space 4
  fpu_s3:
    .space 4
  fpu_s4:
    .space 4
  fpu_s5:
    .space 4
  fpu_s6:
    .space 4
  fpu_s7:
    .space 4
  fpu_s8:
    .space 4
  fpu_s9:
    .space 4
  fpu_s10:
    .space 4
  fpu_s11:
    .space 4
  fpu_s12:
    .space 4
  fpu_s13:
    .space 4
  fpu_s14:
    .space 4
  fpu_s15:
    .space 4

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
    jmp panic ; code de sortie = 100 + LD_*

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
    clr r0
    clr r1
    clr r2
    clr r3
    clr r4
    clr r5
    clr r6
    clr r7
    clr r8
    clr r9
    clr r10
    clr fp
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
    cmpi r0, SYS_GETPID
    jz sys_getpid
    cmpi r0, SYS_YIELD
    jz sys_yield
    cmpi r0, SYS_SPAWNBG
    jz sys_spawnbg
    cmpi r0, SYS_WAIT
    jz sys_wait
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
    sbasew fp, r4, P_FP
    ldw r5, cur_pid
    shli r5, r5, 6
    leab r6, fpu_tbl
    add r6, r6, r5 ; r6 = slot FPU du processus
    fstw f0, fpu_s0
    fstw f1, fpu_s1
    fstw f2, fpu_s2
    fstw f3, fpu_s3
    fstw f4, fpu_s4
    fstw f5, fpu_s5
    fstw f6, fpu_s6
    fstw f7, fpu_s7
    fstw f8, fpu_s8
    fstw f9, fpu_s9
    fstw f10, fpu_s10
    fstw f11, fpu_s11
    fstw f12, fpu_s12
    fstw f13, fpu_s13
    fstw f14, fpu_s14
    fstw f15, fpu_s15
    leab r5, fpu_s0
    memcpy r6, r5, 64
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
    lbasew fp, r4, P_FP
    ldw r5, cur_pid
    shli r5, r5, 6
    leab r6, fpu_tbl
    add r6, r6, r5 ; r6 = slot FPU du processus
    leab r5, fpu_s0
    memcpy r5, r6, 64
    fldw f0, fpu_s0
    fldw f1, fpu_s1
    fldw f2, fpu_s2
    fldw f3, fpu_s3
    fldw f4, fpu_s4
    fldw f5, fpu_s5
    fldw f6, fpu_s6
    fldw f7, fpu_s7
    fldw f8, fpu_s8
    fldw f9, fpu_s9
    fldw f10, fpu_s10
    fldw f11, fpu_s11
    fldw f12, fpu_s12
    fldw f13, fpu_s13
    fldw f14, fpu_s14
    fldw f15, fpu_s15
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

  ; run_pid : r1 = pid d'un processus P_READY (ou d'un fils tout juste créé).
  ; Il devient le processus courant et reprend avec r0 = P_RET. Ne revient pas.
  run_pid:
    stw r1, cur_pid
    PCB_OF(r4, r1, r5)
    movi r5, P_RUN
    sbasew r5, r4, P_STATE
    lbasew r5, r4, P_BASE
    stw r5, cur_base
    lbasew r5, r4, P_TOP
    stw r5, cur_top
    lbasew r0, r4, P_RET
    jmp resume_cur

  ; schedule : prochain processus P_READY, en tourniquet à partir de cur_pid + 1.
  ; Ne revient pas. Aucun processus prêt (interblocage) : halt avec le code 111.
  schedule:
    ldw r1, cur_pid
    movi r2, PROC_MAX

  sch_loop:
    cmpi r2, 0
    jz sch_none
    inc r1
    cmpi r1, PROC_MAX
    jl sch_nowrap
    clr r1

  sch_nowrap:
    PCB_OF(r4, r1, r5)
    lbasew r5, r4, P_STATE
    cmpi r5, P_READY
    jz run_pid ; r1 = pid
    dec r2
    jmp sch_loop

  sch_none:
    movi r0, 111
    halt

  ; proc_exit : r1 = code de sortie. Termine le processus courant. Ne revient pas.
  ;   - pid 0 (racine) : arrêt de la machine, r0 = code
  ;   - ses fils : zombies libérés, vivants orphelins (P_PARENT = P_NONE, libérés à leur sortie)
  ;   - son parent attend ce fils (SPAWN ou WAIT) : il est réveillé avec le code ;
  ;     sinon le processus devient ZOMBIE jusqu'au WAIT du parent
  proc_exit:
    mov r7, r1 ; code
    ldw r8, cur_pid
    cmpi r8, 0
    jz root_exit
    PCB_OF(r4, r8, r5) ; r4 = PCB du mourant
    lbasew r1, r4, P_BASE ; on efface la région : rien ne fuit vers le prochain occupant
    lbasew r2, r4, P_TOP
    call zero_region ; préserve r4

    clr r9
  pe_kids:
    cmpi r9, PROC_MAX
    jz pe_kids_done
    PCB_OF(r5, r9, r6)
    lbasew r6, r5, P_STATE
    cmpi r6, P_FREE
    jz pe_kids_next
    lbasew r6, r5, P_PARENT
    cmp r6, r8
    jnz pe_kids_next
    lbasew r6, r5, P_STATE
    cmpi r6, P_ZOMBIE
    jnz pe_orphan
    movi r6, P_FREE
    sbasew r6, r5, P_STATE
    jmp pe_kids_next

  pe_orphan:
    movi r6, P_NONE
    sbasew r6, r5, P_PARENT

  pe_kids_next:
    inc r9
    jmp pe_kids

  pe_kids_done:
    lbasew r9, r4, P_PARENT
    cmpi r9, 0
    jl pe_free ; orphelin : personne n'attend ce code
    PCB_OF(r5, r9, r6) ; r5 = PCB du parent
    lbasew r6, r5, P_STATE
    cmpi r6, P_WAIT
    jnz pe_zombie
    lbasew r6, r5, P_WFOR
    cmpi r6, 0
    jl pe_any ; il attend n'importe quel fils
    cmp r6, r8
    jnz pe_zombie ; il attend un autre fils
    jmp pe_deliver

  pe_any:
    sbasew r8, r5, P_R1 ; WAIT(-1) : r1 = pid du fils terminé

  pe_deliver:
    sbasew r7, r5, P_RET ; valeur de retour de son SPAWN / WAIT
    movi r6, P_READY
    sbasew r6, r5, P_STATE
    jmp pe_free

  pe_zombie:
    sbasew r7, r4, P_EXIT
    movi r6, P_ZOMBIE
    sbasew r6, r4, P_STATE
    jmp schedule

  pe_free:
    movi r6, P_FREE
    sbasew r6, r4, P_STATE
    jmp schedule

  root_exit:
    mov r0, r7 ; EnvironmentManager::start() lit r0
    halt

  ; spawn_common : r1 = indice du programme
  ;   -> r0 = pid du fils (état P_READY, contexte initial à zéro) ou erreur < 0
  ;   À appeler APRÈS ctx_save : utilise r1..r10 librement.
  spawn_common:
    mov r7, r1
    ldw r1, cur_pid
    call proc_alloc ; r0 = pid, r1 = base, r2 = sommet
    cmpi r0, 0
    jl sc_noslot
    mov r8, r0 ; pid du fils
    mov r9, r1 ; base du fils
    mov r10, r2 ; sommet du fils
    mov r1, r7
    mov r2, r9
    mov r3, r10
    call exec_load ; r0 = LD_*, r1 = entrée, r2 = sommet, r3 = taille de pile
    cmpi r0, 0
    jnz sc_failed
    PCB_OF(r4, r8, r5) ; PCB du fils : contexte de départ
    ori r1, r1, 1
    sbasew r1, r4, P_PC
    sbasew r2, r4, P_USP
    sub r3, r2, r3
    sbasew r3, r4, P_SLIM
    clr r5
    movi r6, P_CMP

  sc_zctx:
    sregw r5, r4, r6
    addi r6, r6, 4
    cmpi r6, P_ZERO_END
    jl sc_zctx
    shli r1, r8, 6 ; registres flottants à zéro
    leab r2, fpu_tbl
    add r1, r1, r2
    addi r2, r1, 64
    call zero_region ; préserve r4
    movi r5, P_READY
    sbasew r5, r4, P_STATE
    mov r0, r8
    ret

  sc_noslot:
    movi r0, SYS_ENOMEM
    ret

  sc_failed: ; le slot a pu être écrit à moitié
    call ld_errno ; r0 = erreur
    mov r1, r9
    mov r2, r10
    call zero_region ; ne touche pas r0
    PCB_OF(r5, r8, r6)
    movi r6, P_FREE
    sbasew r6, r5, P_STATE
    ret

  ; sys_spawn : r1 = indice du programme. Le parent attend la fin de CE fils.
  ;   -> r0 = code de sortie du fils ou erreur < 0
  sys_spawn:
    call ctx_save ; contexte du parent sauvegardé (r1 inchangé)
    call spawn_common
    cmpi r0, 0
    jl spawn_fail ; le parent n'a pas été touché
    ldw r5, cur_pid
    PCB_OF(r4, r5, r6)
    movi r6, P_WAIT
    sbasew r6, r4, P_STATE
    sbasew r0, r4, P_WFOR ; attend ce fils précis
    mov r1, r0
    jmp run_pid ; le fils tourne immédiatement

  spawn_fail:
    jmp resume_cur ; le parent reprend avec r0 < 0 et tous ses registres

  ; sys_spawnbg : r1 = indice du programme. Le fils tourne en arrière-plan (P_READY) :
  ;   il ne s'exécute que quand le parent attend, cède la main ou se termine.
  ;   -> r0 = pid du fils ou erreur < 0
  sys_spawnbg:
    call ctx_save
    call spawn_common
    jmp resume_cur

  sys_getpid:
    ldw r0, cur_pid
    jmp trap_exit

  sys_yield:
    call ctx_save
    ldw r1, cur_pid
    PCB_OF(r4, r1, r5)
    clr r0
    sbasew r0, r4, P_RET
    movi r5, P_READY
    sbasew r5, r4, P_STATE
    jmp schedule ; peut très bien réélire ce même processus

  ; sys_wait : r1 = pid voulu (-1 = n'importe quel fils)
  ;   -> r0 = code de sortie, r1 = pid du fils ; SYS_ECHILD si aucun fils ne correspond.
  ;   Bloque (P_WAIT) tant qu'aucun fils correspondant n'est terminé.
  sys_wait:
    call ctx_save
    mov r7, r1 ; pid voulu
    ldw r8, cur_pid
    clr r9 ; indice du slot examiné
    clr r10 ; 1 si au moins un fils correspond

  wt_loop:
    cmpi r9, PROC_MAX
    jz wt_scanned
    PCB_OF(r4, r9, r5)
    lbasew r5, r4, P_STATE
    cmpi r5, P_FREE
    jz wt_next
    lbasew r5, r4, P_PARENT
    cmp r5, r8
    jnz wt_next
    cmpi r7, 0
    jl wt_match ; -1 : n'importe quel fils
    cmp r7, r9
    jnz wt_next

  wt_match:
    movi r10, 1
    lbasew r5, r4, P_STATE
    cmpi r5, P_ZOMBIE
    jz wt_reap

  wt_next:
    inc r9
    jmp wt_loop

  wt_reap: ; zombie : on récupère son code et on libère sa PCB
    lbasew r0, r4, P_EXIT
    movi r5, P_FREE
    sbasew r5, r4, P_STATE
    PCB_OF(r4, r8, r5) ; PCB du parent (moi)
    sbasew r9, r4, P_R1 ; r1 = pid du fils
    jmp resume_cur

  wt_scanned:
    cmpi r10, 0
    jnz wt_block
    movi r0, SYS_ECHILD
    jmp resume_cur

  wt_block: ; des fils existent mais aucun n'est terminé : on s'endort
    PCB_OF(r4, r8, r5)
    sbasew r7, r4, P_WFOR
    movi r5, P_WAIT
    sbasew r5, r4, P_STATE
    jmp schedule
