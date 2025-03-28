.text
// Caso 1: ADD inmediato sin shift (X0 = X1 + 5)
movz X1, #5
add X0, X1, #3         // X0 = 5 + 3 = 8

// Caso 2: ADD inmediato con shift = 1 (X2 = X1 + (2 << 12) = 5 + 8192)
add X2, X1, #2, LSL #12  // X2 = 5 + 8192 = 8197

// Caso 3: ADD registro (X3 = X0 + X2)
add X3, X0, X2         // X3 = 8 + 8197 = 8205

hlt 0
