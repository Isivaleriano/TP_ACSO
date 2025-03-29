.text
    movz X1, 10       // X1 = 10
    movz X2, 5        // X2 = 5
    cmp X1, X2        // 10 - 5 = positivo → FLAG_N = 0
    bge salto         // salta
    movz X3, 99       // se saltea esta
salto:
    movz X3, 42
    hlt 0
