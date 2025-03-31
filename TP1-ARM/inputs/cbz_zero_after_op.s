.text
    movz X1, 2
    subs X1, X1, #2      // X1 = 0
    subs X1, X1, #2      
    cbz X1, done
    movz X2, 999        
done:
    movz X2, 1
    hlt 0
