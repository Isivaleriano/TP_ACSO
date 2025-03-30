.text
movz X1, #0xAB                // X1 = 0xAB
movz X2, #0x1000, lsl #16     // X2 = 0x10000000
stur X1, [X2, #0x10]          // Guardar los 32 bits de X1 en memoria
ldurb X3, [X2, #0x10]         // Cargar solo los primeros 8 bits (0xAB)
hlt 0
