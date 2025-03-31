.text
movz X1, 10
cmp X1, #5         
bne salto
movz X2, 99        
salto:
movz X2, 42
hlt 0
