.text
movz X0, 5
movz X1, 2

cmp X0, X1
bgt foo

adds X2, X2, 1

bar: 
hlt 0

foo:
adds X3, X3, 99
b bar
