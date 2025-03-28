.text
// Inicializamos X1 = 3 y X2 = 7
movz X1, #3
movz X2, #7

// X0 = X1 * X2 = 3 * 7 = 21
mul X0, X1, X2

hlt 0
