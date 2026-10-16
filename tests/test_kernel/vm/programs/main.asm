%include "syscalls.inc"

.entry main

.section .text

  print_nl:
    movi r0, SYS_WRITE
    leab r1, nl_char
    movi r2, 1
    syscall
    ret

  str_starts_with:
    ; r0 = &buf1, r1 = &buf2, r2 = len(buf1), r3 = len(buf2), r4 = output(1 or 0)
    cmp r2, r3 ; if r2 < r3: return 0
    jl ssw_ret_f
    min r2, r2, r3 ; = min(len(buf1), len(buf2))
    clr r3 ; i = 0
    ssw_loop: ; do
      lregb r5, r0, r3
      lregb r6, r1, r3
      cmp r5, r6 ; if buf1[i] != buf2[i]
      jnz ssw_ret_f ; return 0
      inc r3 ; i++
      cmp r3, r2 ; while i < min
      jl ssw_loop
    jmp ssw_ret_t
    ssw_ret_f:
      movi r4, 0
      ret
    ssw_ret_t:
      movi r4, 1
      ret

  int_to_str:
    ; r0 = input, its_buf = output
    clr r1 ; i
    clr r2 ; j
    clr r3 ; value
    leab r5, its_buf ; buf
    leab r6, temp_buf ; temp

    cmpi r0, 0
    jg its_cond0
    jz its_cond0
      movi r4, '-'
      sregb r4, r5, r2
      inc r2
    its_cond0:
      mov r3, r0
    its_cond1:
      mov r4, r3
      modi r4, r4, 10
      addi r4, r4, 48
      sregb r4, r6, r1
      inc r1
      divi r3, r3, 10
      cmpi r3, 0
      jnz its_cond1
    its_cond2:
      dec r1
      lregb r4, r6, r1
      sregb r4, r5, r2
      inc r2
      cmpi r1, 0
      jg its_cond2
    movi r4, '\0'
    sregb r4, r5, r2
    stw r2, its_buf_size
    ret

  main:
    movi r0, SYS_WRITE
    leab r1, init_msg
    movi r2, 88
    syscall
    console_loop:
      ; print(prompt) # "> "
      movi r0, SYS_WRITE
      leab r1, prompt
      movi r2, 2
      syscall

      ; input_buf = input()
      movi r0, SYS_READ
      leab r1, input_buf
      movi r2, 64
      syscall
      ; buf_size = string(len(input_buf))
      stw r0, buf_size
      mov r2, r0 ; nombre d'octets lus
      call int_to_str

      ; print(o_read_0, len(input_buf), o_read_1) # (octets lus ... ) \n
      movi r0, SYS_WRITE
      leab r1, o_read_0
      movi r2, 13
      syscall
      movi r0, SYS_WRITE
      leab r1, its_buf
      ldw r2, its_buf_size
      syscall
      movi r0, SYS_WRITE
      leab r1, o_read_1
      movi r2, 3
      syscall

      ; print(input_buf)
      movi r0, SYS_WRITE
      leab r1, input_buf
      ldw r2, buf_size
      syscall
      ; print("\n")
      call print_nl
      ; if input_buf.starts_with("help")
      leab r0, input_buf
      leab r1, help_str
      ldw r2, buf_size
      movi r3, 4
      call str_starts_with
      mov cmp, r4
      jz no_help
      movi r0, SYS_WRITE
      leab r1, help_msg
      movi r2, 51
      syscall
      no_help:
        clr r4
      ; if input_buf.starts_with("exit")
      leab r0, input_buf
      leab r1, exit_str
      ldw r2, buf_size
      movi r3, 4
      call str_starts_with
      mov cmp, r4
      jz no_exit
      movi r0, SYS_WRITE
      leab r1, exit_msg
      movi r2, 30
      syscall
      call exit
      no_exit:
        clr r4

      jmp console_loop

  exit:
    movi r0, SYS_EXIT
    movi r1, 0
    syscall
    ret


.section .data
  buf_size:
    .word 0
  its_buf_size:
    .word 0
.section .rodata
  prompt:
    .asciz "> "
  init_msg:
    .asciz "=========== MATHIS OS =========== \n Maximum input size is 64 bytes. Type help for some.\n"
  o_read_0:
    .asciz "(octets lus: "
  o_read_1:
    .asciz ") \n command is: "
  nl_char:
    .asciz "\n"
  help_msg:
    .asciz "Degage de mon OS. Allez tape exit pour t'en aller \n"
  help_str:
    .asciz "help"
  exit_msg:
    .asciz "Oui vas-y c'est pas trop tot \n"
  exit_str:
    .asciz "exit"

.section .bss
  input_buf:
    .space 64
  its_buf:
    .space 64
  temp_buf:
    .space 64