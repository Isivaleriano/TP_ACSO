.text
    movz X3, #1        
    cbnz X3, label    
    movz X0, #42      
    b end

label:
    movz X0, #99       

end:
    hlt 0
