movz X3, #0x5678, lsl #32    // X3 = 0x567800000000
movz X3, #0x1234, lsl #48    // X3 = 0x1234567800000000 ← overwrite zona alta
hlt 0
