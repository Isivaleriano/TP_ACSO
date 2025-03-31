.text
movz X0, 9
movz X1, 2
cmp X0, X1
bge destino
adds X2, X2, 10   
destino:
adds X3, X3, 99   
hlt 0
