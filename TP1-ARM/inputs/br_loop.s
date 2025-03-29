.text
movz X3, 3              // contador
movz X4, 0x000C         // offset a la instrucción `adds`
add X4, X4, #0x400000   // X4 = dirección del loop
movz X5, 0              // acumulador
br X4

loop:
adds X5, X5, #1
subs X3, X3, #1
cmp X3, #0
bne loop
hlt 0
