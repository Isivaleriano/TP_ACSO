.text
movz X1, 10
cmp X1, #5         // 10 - 5 → Z = 0
bne salto
movz X2, 99        // esta no debe ejecutarse
salto:
movz X2, 42
hlt 0
