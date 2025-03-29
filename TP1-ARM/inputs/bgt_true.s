.text
movz X1, 10
cmp X1, #5          // 10 - 5 → Z = 0, N = 0 → BGT salta
bgt mayor
movz X2, 99         // esta se saltea
mayor:
movz X2, 42
hlt 0
