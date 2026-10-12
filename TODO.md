
===================== FAIT =====================

[x] Fetch depuis la RAM (format d'instruction fixe 8o, PC = adresse octet)
[x] ABI syscall + trap (trap_vector, mode USER/KERNEL, SYSRET)
[x] Séparation USER/KERNEL (faute si MMIO touché ou instr privilégiée en USER)
[x] DISK_READ/DISK_WRITE réels sur hard_drive (StorageDevice + registres MMIO)
[x] Infrastructure de boot (ROM séparée, build_rom/build_image/flash_disk, base_address dans link())
[x] boot.asm : boucle DISK_READ + saut indirect (push/ret) vers le kernel
[x] kernel.asm : dispatcher de syscalls (lit r0, route, écrit r0, sysret)
[x] Storage driver (asm) au-dessus des registres DISK_*
[x] superblocs
[x] Format executable

===================== KERNEL / BOOT (C++ prêt, reste l'assembleur) =====================
1. Timer IRQ + préemption (InterruptController est un stub vide dans devices.h)
2. Allocation de blocs
3. Inodes
4. Répertoires
5. Création/lecture/écriture de fichiers
6. File descriptors
7. Process structure
8. Scheduler
9. Plusieurs Core dans CPU (multi-coeur) -> voir note plus bas
10. SLEEP
11. EXEC / EXIT / WAIT

===================== ASSEMBLEUR / OUTILLAGE =====================
27. CHANGER sys_time (TimerDevice, devices.h) en prévision du bug 2038

===================== NETTOYAGE (mineur, pas urgent) =====================
- run_handler.h : RunResult::SYSCALL_STOP pas utilisé

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

===================== STORAGE =====================

bloc 0        superblock
bloc 1        table des exécutables (16 entrées × 32 o)
blocs 2..K    image kernel
blocs K+1..   exécutables : [header 512 o][image paddée à 512 o]

boot.asm (ROM) ─ lit le superblock ─► charge le kernel
kernel.asm ─► loader.asm (exec_load) ─► storage.asm (disk_read / disk_write) ─► MMIO

------------------------------------------------------------------------------------------------------------------------

1. Storage device
2. Storage driver
3. Format du disque
5. Allocation de blocs
6. Inodes
7. Répertoires
8. Création/lecture/écriture de fichiers
9. File descriptors
10. Executable loader
11. Process structure
13. Scheduler
14. Interruptions timer

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