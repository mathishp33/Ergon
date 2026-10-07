

bon j'avoue que j'ai donné le TODO à refaire par ChatGPT... (j'avais trop la flemme)
j'ai quand meme rédigé les parties importantes :-)

===================== FAIT =====================

[x] Fetch depuis la RAM (format d'instruction fixe 8o, PC = adresse octet)
[x] ABI syscall + trap (trap_vector, mode USER/KERNEL, SYSRET)
[x] Séparation USER/KERNEL (faute si MMIO touché ou instr privilégiée en USER)
[x] DISK_READ/DISK_WRITE réels sur hard_drive (StorageDevice + registres MMIO)
[x] Infrastructure de boot (ROM séparée, build_rom/build_image/flash_disk, base_address dans link())
[x] boot.asm : boucle DISK_READ + saut indirect (push/ret) vers le kernel
[x] kernel.asm : dispatcher de syscalls (lit r0, route, écrit r0, sysret)

===================== KERNEL / BOOT (C++ prêt, reste l'assembleur) =====================
1. Timer IRQ + préemption (InterruptController est un stub vide dans devices.h)
4. Storage driver (asm) au-dessus des registres DISK_* (*)
5. Format de disque minimal (superblock) (*)
6. Allocation de blocs
7. Inodes
8. Répertoires
9. Création/lecture/écriture de fichiers
10. File descriptors
11. Format exécutable Ergon + executable loader (*)
12. Process structure
13. Scheduler
14. Plusieurs Core dans CPU (multi-coeur) -> voir note plus bas
15. SLEEP
16. EXEC / EXIT / WAIT

===================== ASSEMBLEUR / OUTILLAGE =====================
25. AJOUTER %include (*)
26. UPDATE le readme
27. CHANGER sys_time (TimerDevice, devices.h) en prévision du bug 2038

===================== NETTOYAGE (mineur, pas urgent) =====================
- run_handler.h : RunResult::SYSCALL_STOP n'est plus jamais retourné
  (callback C++ supprimé), et #include <functional> n'est plus utilisé.

===================== NOTE : plusieurs Core dans CPU =====================
Actuellement SimpleCPU ne contient qu'un seul SimpleCore. Pour du
multi-coeur : SimpleCPU garderait un tableau/vector de SimpleCore (chacun
avec ses propres regs/PC/SP/mode/trap_vector) partageant le même
SystemBus (donc la même RAM/ROM/MMIO). run()/step() prendraient un index
de core. Implique aussi une synchronisation des accès concurrents au bus
(aucun verrou actuellement) et un scheduler qui répartit les processus
entre coeurs plutôt qu'un seul. À faire après un scheduler mono-coeur qui
fonctionne déjà : le multi-coeur ajoute de la concurrence par-dessus un
problème déjà pas trivial.
-> Vecteur/Array de Core dans CPU partageant le meme SystemBus
-> run() prend un index de core
-> synchronisation/blocage d'accès au SystemBus 
-> scheduler mono-coeur puis multi-coeur

===================== NOTE : pour plus tard =====================
Ajouter GPU, Ecran, ... (Graphiques)
Ajouter SoundCard
Ajouter INTERNET !!!

------------------------------------------------------------------------------------------------------------------------

1. Storage device
2. Storage driver
3. Format du disque
4. Superblock
5. Allocation de blocs
6. Inodes
7. Répertoires
8. Création/lecture/écriture de fichiers
9. File descriptors
10. Executable loader
11. Process structure
12. SYSCALL/traps
13. Scheduler
14. Interruptions timer
15. USER/KERNEL privilege separation

------------------------------------------------------------------------------------------------------------------------

A sample system memory map
Address range (hexadecimal)	Size	Device
0000–7FFF	32 KiB	RAM
8000–80FF	256 bytes	General-purpose I/O
9000–90FF	256 bytes	Sound controller
A000–A7FF	2 KiB	Video controller/text-mapped display RAM
C000–FFFF	16 KiB	ROM
TOTAL = FFFF

------------------------------------------------------------------------------------------------------------------------

① Storage virtuel brut\
↓\
② DISK_READ / DISK_WRITE\
↓\
③ format de disque minimal\
↓\
④ filesystem minimal\
↓\
⑤ fichiers\
↓\
⑥ format exécutable Ergon\
↓\
⑦ loader\
↓\
⑧ processus\
↓\
⑨ scheduler\
↓\
⑩ SLEEP\
↓\
⑪ séparation USER/KERNEL\
↓\
⑫ EXEC / EXIT / WAIT\

------------------------------------------------------------------------------------------------------------------------

┌─────────────────────────────┐\
│ Kernel                      │\
│                             │\
│ boot                        │\
│ memory manager              │\
│ process manager             │\
│ scheduler                   │\
│ syscall handler             │\
│ drivers                     │\
│ filesystem                  │\
│ executable loader           │\
└─────────────────────────────┘\

------------------------------------------------------------------------------------------------------------------------

Programme Ergon\
│\
│ SYSCALL\
▼\
CPU / VM\
│\
│ trap\
▼\
Kernel Ergon\
│\
│ traite le syscall\
▼\
driver / filesystem / scheduler\

------------------------------------------------------------------------------------------------------------------------

USER program\
│\
│ SYSCALL\
▼\
┌──────────────┐\
│ CPU          │\
│              │\
│ mode USER    │\
└──────┬───────┘\
│\
│ trap\
▼\
┌──────────────┐\
│ Kernel       │\
│              │\
│ mode KERNEL  │\
└──────────────┘\

ATTENTION AUX MECANISMES DE TRAP !!!!!\
SEUL LE KERNEL DOIT POSSEDER LES DRIVERS


Executable loader\
↓\
Filesystem\
↓\
Storage driver\
↓\
StorageDevice\


USER PROGRAM\
│\
│ open/read/exec\
▼\
KERNEL\
├── syscall handler\
├── process manager\
├── filesystem\
├── executable loader\
├── storage driver\
│\
▼\
VM Storage Device\
│\
▼\
persistent disk\

------------------------------------------------------------------------------------------------------------------------

VM\
│\
├── ROM\
│    └── boot firmware\
│\
├── RAM\
│\
└── Storage\
└── kernel executable\

------------------------------------------------------------------------------------------------------------------------

RESET\
↓\
ROM boot code\
↓\
initialisation hardware\
↓\
initialisation RAM\
↓\
initialisation storage\
↓\
charger kernel depuis storage\
↓\
kernel → RAM\
↓\
jump kernel entry\

------------------------------------------------------------------------------------------------------------------------

┌────────────────────────────────────┐\
│          USER PROGRAMS             │\
│                                    │\
│ shell / applications / games       │\
└────────────────┬───────────────────┘\
│\
SYSCALL\
│\
▼\
┌────────────────────────────────────┐\
│              KERNEL                │\
│                                    │\
│ syscall handler                    │\
│ scheduler                          │\
│ process manager                    │\
│ memory manager                     │\
│ filesystem                         │\
│ drivers                            │\
│ executable loader                  │\
└────────────────┬───────────────────┘\
│\
hardware access\
│\
▼\
┌────────────────────────────────────┐\
│                VM                  │\
│                                    │\
│ CPU                                │\
│ RAM                                │\
│ ROM                                │\
│ Storage                            │\
│ timers / devices                   │\
└────────────────────────────────────┘\