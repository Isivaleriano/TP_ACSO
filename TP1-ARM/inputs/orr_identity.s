.text
movz X1, 0xABCD       // X1 = 0000...ABCD
movz X2, 0x0000       // X2 = 0x0
orr X3, X1, X2        // X3 = X1
hlt 0
