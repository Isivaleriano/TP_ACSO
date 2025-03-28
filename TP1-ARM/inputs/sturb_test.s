.text
movz X1, #0xAB     // Valor a guardar
movz X2, #0x1000, lsl #16 // Dirección base: 0x10000000
sturb W1, [X2, #0x10]     // Escribe 0xAB en 0x10000010
hlt 0

