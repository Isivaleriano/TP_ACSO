.text
movz X3, 0
loop:
adds X3, X3, #1   
cmp X3, #3
bne loop
hlt 0
