        .text
        .global _start

_start:
        adds X0, X0, #1          // X0 = 1
        b skip             // salta a 'skip', se saltea la suma siguiente
        adds X0, X0, #100  // esta NO se debe ejecutar (salteada)

skip:
        adds X1, X0, #5    // X1 = X0 + 5 (debería dar 6)
        hlt 0