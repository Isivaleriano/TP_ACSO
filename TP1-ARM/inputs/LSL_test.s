.text
.global _start

_start:
    movz X3, #5         // X3 = 5
    lsl X4, X3, #2      // X4 = 20
    hlt #0
    