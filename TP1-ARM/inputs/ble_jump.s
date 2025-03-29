.text
    movz X1, 5        // X1 = 5
    movz X2, 10       // X2 = 10
    cmp X1, X2        // 5 - 10 = negativo → FLAG_N = 1
    ble salto         // salta
    movz X3, 99       // se saltea esta
salto:
    movz X3, 42
    hlt 0
