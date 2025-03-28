.text
movz X1, 0xFFFF       // X1 = 0000...FFFF
movz X2, 0x1234       // X2 = 0000...1234
orr X3, X1, X2        // X3 = 0xFFFF
hlt 0
