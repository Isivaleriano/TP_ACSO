.text
    movz X3, #1        // X3 = 1 (distinto de cero)
    cbnz X3, label     // Salta porque X3 != 0
    movz X0, #42       // Esto se saltea
    b end

label:
    movz X0, #99       // Este se ejecuta si X3 != 0

end:
    hlt 0
