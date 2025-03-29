.text
    movz X1, 0
    cbz X1, label
    movz X2, 999        // Esto no debe ejecutarse
label:
    movz X2, 42
    hlt 0
