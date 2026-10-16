%equ SYS_EXIT 0 ; r1 = code
%equ SYS_WRITE 1 ; r1 = ptr, r2 = len -> r0 = octets écrits
%equ SYS_READ 2 ; r1 = ptr, r2 = len -> r0 = octets lus (0 = EOF)
%equ SYS_EXEC 3 ; r1 = indice de la table -> ne revient pas si OK, sinon r0 < 0
%equ SYS_SPAWN 4 ; r1 = indice -> r0 = code de sortie du fils (0..255) ou erreur < 0

%equ SYS_ENOSYS -1
%equ SYS_EFAULT -2
%equ SYS_ENOENT -3
%equ SYS_EBADEXEC -4
%equ SYS_EIO -5
%equ SYS_ENOMEM -6