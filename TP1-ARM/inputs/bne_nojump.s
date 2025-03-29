.text
movz X1, 5
cmp X1, #5         // 5 - 5 → Z = 1
bne salto
movz X2, 42        // esta sí se ejecuta
salto:
movz X3, 99
hlt 0
