.text
    movz X1, 0
    cbnz X1, label
    movz X2, 33         
    b end
label:
    movz X2, 99
end:
    hlt 0
