.text
movz X1, 3
cmp X1, #8          // 3 - 8 → negativo → N = 1 → no salta
bgt mayor
movz X2, 42         // esta sí se ejecuta
mayor:
movz X3, 99
hlt 0
