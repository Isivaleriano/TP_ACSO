.text
movz X3, #0         // X3 = 0
cbz X3, etiqueta    // Debería saltar

movz X0, #123       // Esto no se ejecuta si salta correctamente
b end

etiqueta:
movz X1, #999       // Esto se ejecuta si X3 era 0

end:
hlt 0

