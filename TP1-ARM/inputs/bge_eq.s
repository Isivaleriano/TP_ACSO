.text
    movz X1, 20
    movz X2, 20
    cmp X1, X2       
    bge salto         
    movz X3, 99
salto:
    movz X3, 123
    hlt 0
