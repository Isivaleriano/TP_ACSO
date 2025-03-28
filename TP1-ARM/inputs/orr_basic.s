.text
movz X1, 0x000F       // X1 = 0000...000F
movz X2, 0x00F0       // X2 = 0000...00F0
orr X3, X1, X2        // X3 = 00FF
hlt 0
