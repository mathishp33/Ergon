VOIR TUTOS sur www.tutorialspoint.com/assembly_programming


bon j'avoue que j'ai donné le TODO à refaire par ChatGPT... (j'avais trop le flemme)

===================== FAIT (vérifié dans le code) =====================

[x] Fetch depuis la RAM (format d'instruction fixe 8o, PC = adresse octet)
[x] ABI syscall + trap (trap_vector, mode USER/KERNEL, SYSRET)
[x] Suppression du handle_syscall côté C++ : SYSCALL trap toujours vers le kernel
[x] Séparation USER/KERNEL (faute si MMIO touché ou instr privilégiée en USER)
[x] DISK_READ/DISK_WRITE réels sur hard_drive (StorageDevice + registres MMIO)
[x] Infrastructure de boot (ROM séparée, build_rom/build_image/flash_disk, base_address dans link())

===================== BLOQUANT - À FAIRE EN PRIORITÉ =====================

2. Bugs ALU déjà repérés, TOUJOURS présents dans run_handler.h :
    - OP_CMPUI compare c.regs[instr->rs2] au lieu de (uint32_t) instr->imm
    - OP_MINI/OP_MAXI assignent instr->rs1 / instr->imm (bruts) au lieu de
      c.regs[instr->rs1] / instr->imm (valeurs)

4. mother_board.h::reset() ne remet plus PC à rom_entry_pc (seulement
   ROM_BASE, valeur par défaut, via core.reset()). Ajouter
   "cpu->core.PC = rom_entry_pc;" après cpu->core.reset(...), sinon un
   .entry qui n'est pas la toute première instruction de la ROM casse le boot.

5. step_handler.h n'a pas reçu le même traitement que run_handler.h
   (fetch depuis le bus, PC += INSTR_SIZE). EnvironmentManager::step()
   est commenté en attendant -> plus de debug pas-à-pas pour le moment.

===================== KERNEL / BOOT (C++ prêt, reste l'assembleur) =====================
6. Timer IRQ + préemption (InterruptController est un stub vide dans devices.h)
7. boot.asm : boucle DISK_READ + saut indirect (push/ret) vers le kernel
8. kernel.asm : dispatcher de syscalls (lit r0, route, écrit r0, sysret)
9. Storage driver (asm) au-dessus des registres DISK_*
10. Format de disque minimal (superblock)
11. Allocation de blocs
12. Inodes
13. Répertoires
14. Création/lecture/écriture de fichiers
15. File descriptors
16. Format exécutable Ergon + executable loader
17. Process structure
18. Scheduler
19. Plusieurs Core dans CPU (multi-coeur) -> voir note plus bas
20. SLEEP
21. EXEC / EXIT / WAIT
22. USER/KERNEL : cas restants (HALT/SETTV déjà couverts ; à revoir si
    d'autres opcodes doivent devenir privilégiés une fois le kernel réel écrit)

===================== ASSEMBLEUR / OUTILLAGE =====================
23. AJOUTER %if, %ifdef (stubs vides dans preprocessor.h, non implémentés)
24. AJOUTER %include
25. UPDATE le readme
26. CHANGER sys_time (TimerDevice, devices.h) en prévision du bug 2038

===================== NETTOYAGE (mineur, pas urgent) =====================
- memory.h : Bus/RAMBus/RAM/ROM ne sont plus utilisés nulle part (remplacés
  par bus.h::SystemBus + mother_board.h::rom/ram) -> supprimable.
- run_handler.h : RunResult::SYSCALL_STOP n'est plus jamais retourné
  (callback C++ supprimé), et #include <functional> n'est plus utilisé.
- parser.h : le registre r11 s'appelle "tmp" dans reg_table, mais l'ABI
  syscall/faute l'utilise maintenant pour la cause du trap (fault_cause).
  Pas un bug, mais un handler de trap qui utilise "tmp" pour autre chose
  doit lire sa valeur AVANT de l'écraser.

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