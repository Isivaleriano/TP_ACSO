.text
    movz X1, 7
    movz X2, 7
    cmp X1, X2       
    ble salto        
    movz X3, 99
salto:
    movz X3, 123
    hlt 0
