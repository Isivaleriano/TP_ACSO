        .text
        .global _start

_start:
        adds X0, X0, #1          
        b skip             
        adds X0, X0, #100  

skip:
        adds X1, X0, #5    
        hlt 0