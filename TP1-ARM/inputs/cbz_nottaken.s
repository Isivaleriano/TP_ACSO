.text
    movz X1, 1
    cbz X1, label
    movz X2, 33        
    b end
label:
    movz X2, 99
end:
    hlt 0
