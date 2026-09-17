VOIR TUTOS sur www.tutorialspoint.com/assembly_programming

TODO:
CHANGER le sys_time dans environment_manager en 2038 !!!
UPDATE le readme
AJOUTER %if, %ifdef et %include
AJOUTER Dispatcher pour les system calls
rajouter open/close comme system calls ((open, close) -> donc memoire (SSD) persistant, spawn(thread))
AJOUTER kernel séparé
AJOUTER filesystem
AJOUTER scheduler
AJOUTER drivers
AJOUTER syscall handler (lower levels)

2. ABI syscall + trap (trap_vector, bit de mode, SYSRET).
3. DISK_READ/DISK_WRITE réels sur hard_drive.
4. Boot minimal (ROM → charge kernel → jump).
5. Séparation USER/KERNEL (faults sur MMIO/instructions sensibles en mode user).
6. Timer IRQ + préemption.
7. Le reste (filesystem, scheduler, exec/exit/wait) : pur assembleur.


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