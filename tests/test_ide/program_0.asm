

.section .text
  movi r0, 1 ; syscall = 1 (write)


.section .data
  my_char_buff:
    .asciz "abcd"
  my_size:
    .word 4