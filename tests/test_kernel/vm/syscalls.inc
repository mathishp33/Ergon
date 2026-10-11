%equ SYS_EXIT 0 ; r1 = code
%equ SYS_WRITE 1 ; r1 = ptr, r2 = len -> r0 = octets écrits
%equ SYS_READ 2 ; r1 = ptr, r2 = len -> r0 = octets lus (0 = EOF)

%equ SYS_ENOSYS -1
%equ SYS_EFAULT -2