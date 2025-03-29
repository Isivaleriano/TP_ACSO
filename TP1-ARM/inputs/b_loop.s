.text
movz X3, 0
loop:
adds X3, X3, #1     // X3++
cmp X3, #3          // compara con 3
bne loop            // salta si X3 ≠ 3
hlt 0
