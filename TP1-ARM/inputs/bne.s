.text
    movz X0, 1       
    movz X1, 2        
    cmp  X0, X1       

    bne  salto        

    adds X2, X2, #42  

fin:
    hlt 0             

salto:
    adds X3, X3, #99  
    b fin            
