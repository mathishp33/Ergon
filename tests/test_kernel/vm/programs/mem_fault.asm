.entry main

.section .text
  main:
    movi r1, 0x1000
    lbasew r2, r1, 0 ; hors de la région : faute cause 5
    movi r0, 0
    halt