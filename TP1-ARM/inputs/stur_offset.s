.text
    movz X1, 0x100      // Dirección base
    movz X2, 0xBBBB     // Valor a guardar
    stur X2, [X1, #4]   // Guardar X2 en X1 + 4
    hlt 0
