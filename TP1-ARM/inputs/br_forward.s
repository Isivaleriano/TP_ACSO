.text
movz X1, 0x0018           // X1 = 0x18
add X1, X1, #0x400000     // X1 = 0x400018
br X1
movz X2, 99               // esta NO debe ejecutarse
movz X2, 42
hlt 0
