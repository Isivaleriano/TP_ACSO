.text
    movz X1, #0x1010     // dirección a saltar (válida en MOVZ)
    br X1                // salta a 0x1010
    movz X0, #123        // no se ejecuta

    // dirección 0x1010
    movz X1, #999        // sí se ejecuta
    hlt 0
