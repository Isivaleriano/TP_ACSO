.text
movz x1, #0xAB                  // X1 = 0xAB
movz x2, #0x1000, lsl #16       // X2 = 0x10000000
stur x1, [x2, #0x10]            // Guardar X1 (32 bits) en memoria
ldurb w3, [x2, #0x10]           // Cargar byte a W3
hlt 0
