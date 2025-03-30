.text

start:
    MOVZ X0, 0x40       // Carga el valor 0x400 en X0
    LSL X0, X0, 16      // Desplaza X0 16 bits a la izquierda
    ADDS X0, X0, 20     // Suma 32 a X0 y actualiza las banderas
    BR X0                // Salta a la dirección almacenada en X0
    ADDS X2, X2, 32     // Suma 32 a X2 y actualiza las banderas
    ADDS X1, X1, 64     // Suma 64 a X1 y actualiza las banderas

HLT 0
