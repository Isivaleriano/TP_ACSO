.text
    movz X1, 0x400100        // Dirección base
    movz W2, 0xABCD       // Valor a guardar en W2 (no X2)
    sturh W2, [X1, #0]    // Guarda los 16 bits de W2 en X1 + 0
    hlt 0
