.text
    movz X1, 10       
    movz X2, 5        
    cmp X1, X2        
    ble salto         
    movz X3, 99       
salto:
    movz X3, 42       
    hlt 0
