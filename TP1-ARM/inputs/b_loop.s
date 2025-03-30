.text
    movz X3, 0          // X3 = 0

loop:
    adds X3, X3, #1     // X3++
    movz X4, 3          // X4 = 3
    cmp X3, X4          // compara X3 con 3
    bne loop            // salta si X3 ≠ 3

    hlt 0
