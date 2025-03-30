.text

movz X9, 0x40       // Carga el valor 0x400 en X0
lsl X9, X9, 16      // Desplaza X0 16 bits a la izquierda
adds X9, X9, 20     // Suma 32 a X0 y actualiza las banderas
br X9                // Salta a la dirección almacenada en X0
adds X10, X10, 32     // Suma 32 a X2 y actualiza las banderas
adds X11, X11, 64     // Suma 64 a X1 y actualiza las banderas

hlt 0
