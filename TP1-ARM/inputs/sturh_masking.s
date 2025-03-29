.text
    movz X1, 0x100
    movz W2, 0xFFFF
    sturh W2, [X1, #0]
    movz W2, 0x0000
    sturh W2, [X1, #0]
    hlt 0
