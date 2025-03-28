.text
// Inicializamos X0 = 5, X1 = 2
movz X0, 5
movz X1, 2

// Comparamos X0 - X1 = 3 → Z=0, N=0 → debe saltar a etiqueta foo
cmp X0, X1
bgt foo

// Esto no debería ejecutarse si salta correctamente
adds X2, X2, 1

bar: 
hlt 0

foo:
// Este se ejecuta si el salto funcionó
adds X3, X3, 99
b bar
