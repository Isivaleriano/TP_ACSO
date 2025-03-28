.text
movz X3, 0x40        // X3 = 0x40 (64 decimal → 0b1000000)
lsr X4, X3, 2        // X4 = X3 >> 2 → 0x10 (16 decimal)
hlt 0
