.text
    movz X1, 5        // X1 = 5
    movz X2, 10       // X2 = 10
    cmp X1, X2        // 5 - 10 = negativo → FLAG_N = 1
    bge salto         // no salta
    movz X3, 99       // se ejecuta
salto:
    movz X3, 42       // pisa el valor
    hlt 0
