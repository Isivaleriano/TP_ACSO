.text
    movz X1, 0x100
    movz X2, 0x1234
    stur X2, [X1, #8]     // Guarda en 0x108
    ldur X3, [X1, #8]     // Lee desde 0x108
    hlt 0
