.text
    movz X1, 2
    sub X1, X1, #2      // X1 = 0
    cbz X1, done
    movz X2, 999        // No se ejecuta
done:
    movz X2, 1
    hlt 0
