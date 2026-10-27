%equ SYS_EXIT 0 ; r1 = code
%equ SYS_WRITE 1 ; r1 = ptr, r2 = len -> r0 = octets écrits
%equ SYS_READ 2 ; r1 = ptr, r2 = len -> r0 = octets lus (0 = EOF)
%equ SYS_EXEC 3 ; r1 = indice de la table -> ne revient pas si OK, sinon r0 < 0
%equ SYS_SPAWN 4 ; r1 = indice -> r0 = code de sortie du fils (0..255) ou erreur < 0
%equ SYS_GETPID 5 ; -> r0 = pid
%equ SYS_YIELD 6 ; -> r0 = 0 ; cède le processeur au prochain processus prêt
%equ SYS_SPAWNBG 7 ; r1 = indice -> r0 = pid du fils (il tourne en arrière-plan) ou erreur < 0
%equ SYS_WAIT 8 ; r1 = pid (-1 = n'importe quel fils) -> r0 = code de sortie (0..255) ou SYS_ECHILD, r1 = pid

%equ SYS_ENOSYS -1
%equ SYS_EFAULT -2
%equ SYS_ENOENT -3
%equ SYS_EBADEXEC -4
%equ SYS_EIO -5
%equ SYS_ENOMEM -6
%equ SYS_ECHILD -7