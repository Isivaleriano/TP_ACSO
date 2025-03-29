.text
    movz X1, 1
    add X2, X1, #2, LSL #12   // X2 = 1 + (2 << 12) = 8193
    hlt 0
