.text
    movz X1, 0x100        // Dirección base
    movz W2, 0xAB         // Valor a guardar (8 bits)
    sturb W2, [X1, #0]    // Guarda byte en memoria
    ldurb W3, [X1, #0]    // Carga byte a W3
    hlt 0
