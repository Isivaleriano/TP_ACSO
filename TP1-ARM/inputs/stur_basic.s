.text
    movz X1, 0x100      // Dirección base
    movz X2, 0xAAAA     // Valor a guardar
    stur X2, [X1, #0]   // Guardar X2 en memoria en X1 + 0
    hlt 0
