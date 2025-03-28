.text
    // X0 = 1, X1 = 2 → X0 != X1 → FLAG_Z = 0 → BNE debe saltar

    movz X0, 1        // Cargar 1 en X0
    movz X1, 2        // Cargar 2 en X1
    cmp  X0, X1       // Compara X0 - X1 → FLAG_Z = 0

    bne  salto        // DEBE SALTAR a "salto"

    adds X2, X2, #42  // Esto NO se debe ejecutar

fin:
    hlt 0             // Debe llegar directo acá si NO saltó

salto:
    adds X3, X3, #99  // Esto se debe ejecutar si saltó
    b fin             // Salta al final
