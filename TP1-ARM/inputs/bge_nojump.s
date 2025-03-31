.text
    movz X1, 5       
    movz X2, 10       
    cmp X1, X2       
    bge salto         
    movz X3, 99       
salto:
    movz X3, 42       
    hlt 0
