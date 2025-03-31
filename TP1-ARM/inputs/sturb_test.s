.text
movz X1, #0xAB  
movz X2, #0x1000, lsl #16 
sturb W1, [X2, #0x10]     
hlt 0

