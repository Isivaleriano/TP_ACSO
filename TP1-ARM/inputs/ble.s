.text
movz X0, 4
movz X1, 4
cmp X0, X1
ble continuar
adds X2, X2, 10   
continuar:
adds X3, X3, 99   
hlt 0
