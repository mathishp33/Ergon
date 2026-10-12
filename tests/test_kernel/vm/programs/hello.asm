%include "syscalls.inc"

.entry main

.section .text


  int_to_str:
    ; r0 = input, its_buf = output
    clr r1 ; i
    clr r2 ; j
    clr r3 ; value
    leab r5, its_buf ; buf
    leab r6, its_temp_buf ; temp

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

      jmp console_loop

    movi r0, SYS_EXIT
    movi r1, 0
    syscall


.section .data
  buf_size:
    .word 0
  its_buf_size:
    .word 0
.section .rodata
  prompt:
    .asciz "> "
  o_read_0:
    .asciz "(octets lus: "
  o_read_1:
    .asciz ") \n"

.section .bss
  input_buf:
    .space 64
  its_buf:
    .space 64
  its_temp_buf:
    .space 64