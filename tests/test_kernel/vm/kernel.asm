%equ KSTACK_TOP 0xF000
%equ MMIO_HI 0xF000
%equ SYS_EXIT 0 ; r0 = syscall/retour, r1..r3 = args
%equ SYS_WRITE 1 ; r0 = syscall écrit, r1 = ptr, r2 = len
%equ SYS_READ 2 ; r0 = syscall écrit, r1 = ptr, r2 = len

.section .text
  .entry kmain

  kmain:
    mov r1, sp ; haut de la pile de boot = pile user
    setusp r1
    movi sp, KSTACK_TOP ; le kernel passe sur sa propre pile
    setksp sp
    leab r1, trap_entry
    settv r1

    leab r1, init_main ; entrée en USER : fausse "adresse de retour" + bit 0
    ori r1, r1, 1
    push r1
    sysret

  ; handler de traps
  trap_entry:
    push r4
    push r5
    push r6
    push cmp ; flags de l'utilisateur (cmp du handler les écrase)

    cmpi trp, 0 ; 0 = syscall, 1/2 = faute
    jnz fault_path

    cmpi r0, SYS_EXIT
    jz sys_exit
    cmpi r0, SYS_WRITE
    jz sys_write

    movi r0, -1 ; syscall inconnu
  trap_exit:
    pop cmp
    pop r6
    pop r5
    pop r4
    sysret

  fault_path:
    addi r0, trp, 128 ; code de sortie = 128 + cause
    halt

  ; syscalls
  sys_exit:
    mov r0, r1
    halt

  sys_write:
    movi r4, MMIO_HI
    shli r4, r4, 16 ; r4 = MMIO_BASE (CONSOLE_OUT = +0)
    clr r5 ; i = 0
  write_loop:
    cmpu r5, r2
    jz write_done
    lregb r6, r1, r5 ; r6 = octet user [r1 + i]
    sbaseb r6, r4, 0 ; -> console
    inc r5
    jmp write_loop
  write_done:
      mov r0, r5
      jmp trap_exit

  ; programme USER embarqué (en attendant le loader)
  init_main:
      movi r0, SYS_WRITE
      leab r1, msg
      movi r2, 13
      syscall
      movi r0, SYS_EXIT
      movi r1, 0
      syscall

.section .data
  msg:
    .asciz "hello world"