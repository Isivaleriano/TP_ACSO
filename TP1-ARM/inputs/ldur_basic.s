.text
    movz X1, 0x100        // X1 = 0x100 (dirección base)
    movz X2, 0xABCD       // X2 = 0xABCD (valor a guardar)
    stur X2, [X1, #0]     // guarda X2 (0xABCD) en la dirección 0x100
    ldur X3, [X1, #0]     // debería cargar el valor desde 0x100 a X3
    hlt 0

