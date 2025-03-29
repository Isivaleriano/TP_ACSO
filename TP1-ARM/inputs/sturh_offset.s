.text
    movz X1, 0x100
    movz W2, 0x1234
    sturh W2, [X1, #2]    // Guarda en 0x102
    hlt 0
