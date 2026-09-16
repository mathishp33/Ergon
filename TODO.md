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