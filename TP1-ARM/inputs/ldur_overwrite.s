.text
    movz X1, 0x100
    movz X2, 0x1111
    stur X2, [X1, #0]
    movz X2, 0x2222
    stur X2, [X1, #0]
    ldur X3, [X1, #0]     // Debería leer 0x2222
    hlt 0
