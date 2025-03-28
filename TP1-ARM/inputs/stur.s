.text
movz X1, #0xABCD          // X1 = 0xABCD (dato a guardar)
movz X2, #0x1000, lsl #16 // X2 = 0x10000000 (dirección base)
stur X1, [X2, #0x10]      // M[X2 + 0x10] = X1 → guarda en 0x10000010
hlt 0
