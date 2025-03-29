.text
    movz X1, 0x100
    movz W2, 0x7F
    sturb W2, [X1, #2]    // Guarda en 0x102
    ldurb W3, [X1, #2]    // Carga desde 0x102
    hlt 0
