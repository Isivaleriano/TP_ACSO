.text
    movz X1, 10       // X1 = 10
    movz X2, 5        // X2 = 5
    cmp X1, X2        // 10 - 5 = positivo → FLAG_N = 0
    ble salto         // no salta
    movz X3, 99       // se ejecuta
salto:
    movz X3, 42       // pisa
    hlt 0
