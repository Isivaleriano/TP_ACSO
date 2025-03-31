.text
    movz X3, 0        

loop:
    adds X3, X3, #1     
    movz X4, 3          
    cmp X3, X4          
    bne loop            

    hlt 0
