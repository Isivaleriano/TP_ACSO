.text
    movz X1, 0x100
    movz X2, 0x1111
    stur X2, [X1, #0]   // Guardar 0x1111 en 0x100
    movz X2, 0x2222
    stur X2, [X1, #0]   // Sobrescribe con 0x2222
    hlt 0
