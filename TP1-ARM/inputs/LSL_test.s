.text
.global _start

_start:
    movz X3, #5         
    lsl X4, X3, #2      
    hlt #0
    