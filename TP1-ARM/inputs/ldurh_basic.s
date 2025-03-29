.text
    movz X1, 0x100        // Dirección base
    movz W2, 0xABCD       // Valor a guardar (16 bits)
    sturh W2, [X1, #0]    // Guarda 0xABCD en memoria
    ldurh W3, [X1, #0]    // Carga 16 bits a W3
    hlt 0
