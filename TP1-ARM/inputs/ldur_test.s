movz X1, #0xABCD           // Valor a guardar
movz X2, #0x1000, lsl #16  // Dirección base: 0x10000000
stur X1, [X2, #0x10]       // Guardamos en memoria
ldur X3, [X2, #0x10]       // Cargamos en X3
hlt 0
