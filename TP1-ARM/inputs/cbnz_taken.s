.text
    movz X1, 1
    cbnz X1, label
    movz X2, 999       
label:
    movz X2, 42
    hlt 0
