.text
movz X1, 7
cmp X1, #7          // 7 - 7 → Z = 1 → no salta
bgt mayor
movz X2, 42         // esta sí se ejecuta
mayor:
movz X3, 99
hlt 0
