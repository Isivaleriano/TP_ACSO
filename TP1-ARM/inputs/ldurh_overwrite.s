.text
    movz X1, 0x100
    movz W2, 0x1111
    sturh W2, [X1, #0]    // Escribe 0x1111
    movz W2, 0x2222
    sturh W2, [X1, #0]    // Sobrescribe con 0x2222
    ldurh W3, [X1, #0]    // Debería cargar 0x2222
    hlt 0
