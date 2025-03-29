.text
    movz X1, 0x100
    movz W2, 0xFF
    sturb W2, [X1, #0]    // Escribe 0xFF
    movz W2, 0x11
    sturb W2, [X1, #0]    // Sobrescribe con 0x11
    ldurb W3, [X1, #0]    // Debería cargar 0x11
    hlt 0
