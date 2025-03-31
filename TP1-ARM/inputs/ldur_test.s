.text
movz X1, #0xABCD           
movz X2, #0x1000, lsl #16  
stur X1, [X2, #0x10]       
ldur X3, [X2, #0x10]       
hlt 0
