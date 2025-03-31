.text
movz X3, #0         
cbz X3, etiqueta    

movz X0, #123       
b end

etiqueta:
movz X1, #999       

end:
hlt 0

