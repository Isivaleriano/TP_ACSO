.text
    movz X1, 0x100
    movz W2, 0x1234
    sturh W2, [X1, #4]    // Guarda en 0x104
    ldurh W3, [X1, #4]    // Carga desde 0x104
    hlt 0
