%include "layout.inc"
%include "mmio.inc"
%include "syscalls.inc"

.entry kmain
.extern loader_init
.extern exec_load

.section .bss
  k_user_top:
    .space 4

.section .text
  kmain:
    mov r7, sp ; boot a laissé SP au sommet de la RAM
    stw r7, k_user_top
    movi sp, KERNEL_STACK_TOP ; pile kernel
    setksp sp
    leab r1, trap_entry
    settv r1

    mov r1, r7
    call loader_init
    cmpi r0, 0
    jnz panic
    movi r1, 0 ; programme n°0 de la table
    call exec_load ; r1 = entrée, r2 = sommet de pile user
    cmpi r0, 0
    jnz panic
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
    cmpui r1, USER_BASE
    jl cub_bad
    add r4, r1, r2
    cmpu r4, r1 ; débordement 32 bits
    jl cub_bad
    ldw r5, k_user_top
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