.text
    movz X1, #0xBEEF         // Valor a guardar → 0xBEEF
    movz X2, #0x1000, lsl #16 // Dirección base → 0x10000000
    sturh W1, [X2, #0x10]     // Guardar los primeros 16 bits de X1 en M[0x10000010]
    hlt 0
