#include "decode.h"
#include <stdio.h>

Instruction decode_instruction(uint32_t hex) {
    Instruction inst = {0};
    inst.raw = hex;
    printf("Decoding raw instruction: 0x%08x\n", hex);
    // Detectar HLT directamente
    if (hex == 0xD4400000) {
        inst.opcode = OP_HLT;
        printf("→ Detectado HLT\n");
        return inst;
    }

    if ((hex & 0x7F800000) == 0x53000000) {
        uint32_t immr = (hex >> 16) & 0x3F;
        uint32_t imms = (hex >> 10) & 0x3F;
    
        // LSL es un alias de UBFM con immr = 64 - shift, imms = 63 - shift
        if (immr == (64 - (63 - imms))) {
            printf("→ Detectado LSL (inmediato)\n");
            inst.opcode = OP_LSL;
            inst.rd = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.shift = 63 - imms;
            inst.uses_imm = 1;
            return inst;
        }
    }
            // Detectar LSR (alias de UBFM con immr = shift, imms = 63)
    if ((hex & 0x7F800000) == 0x53000000) {
        uint32_t immr = (hex >> 16) & 0x3F;
        uint32_t imms = (hex >> 10) & 0x3F;

        // LSR: alias de UBFM con immr = shift, imms = 63
        if (imms == 63) {
            printf("→ Detectado LSR (inmediato)\n");
            inst.opcode = OP_LSR;
            inst.rd = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.shift = immr;
            inst.uses_imm = 1;
            return inst;
        }
    }

    if ((hex & 0xFFC00000) == 0xB8000000) {
        printf("→ Detectado STUR (registro)\n");
        inst.opcode = OP_STUR;
        inst.rt = hex & 0x1F;
        inst.rn = (hex >> 5) & 0x1F;
        inst.offset = (hex >> 12) & 0x1FF;  // 9 bits sin signo
        return inst;
    }

    uint32_t opcode = (hex >> 21) & 0x7FF;
    if (opcode >= 0x694 && opcode <= 0x697) {
        printf("→ Detectado MOVZ\n");
        inst.opcode = OP_MOVZ;
        inst.rd = hex & 0x1F;
        inst.imm = (hex >> 5) & 0xFFFF;
        inst.hw = (hex >> 21) & 0x3;
        inst.uses_imm = 1;
        return inst;
    }

    if ((hex & 0x7C000000) == 0x14000000) {
        printf("→ Detectado B (por patrón de bits)\n");
        inst.opcode = OP_B;
    
        int32_t imm26 = hex & 0x03FFFFFF;
    
        // Sign extension: si el bit 25 es 1, extender con 1s a la izquierda
        if (imm26 & (1 << 25)) {
            imm26 |= ~0x03FFFFFF;
        }
    
        inst.branch_offset = imm26 << 2;
        return inst;
    }

    // CBZ / CBNZ detection fix
    if ((hex & 0x7E000000) == 0x34000000) {
        uint32_t rt = hex & 0x1F;
        int32_t offset = (hex >> 5) & 0x7FFFF;
        if (offset & (1 << 18)) {
            offset |= ~0x7FFFF; // sign-extend
        }
        offset <<= 2;
    
        inst.rt = rt; // usamos rt, no rn
        inst.branch_offset = offset;
    
        if ((hex & (1 << 24)) == 0) {
            printf("→ Detectado CBZ\n");
            inst.opcode = OP_CBZ;
        } else {
            printf("→ Detectado CBNZ\n");
            inst.opcode = OP_CBNZ;
        }
        return inst;
    }
    
    

    if ((hex & 0xFF000010) == 0x54000000) {
            uint8_t cond = hex & 0xF;
            int32_t offset = ((hex >> 5) & 0x7FFFF);
            if (offset & (1 << 18)) {
                offset |= ~0x7FFFF; // sign-extend
            }
            offset <<= 2;
            inst.branch_offset = offset;
    
            switch (cond) {
                case 0b0000: // BEQ
                    printf("→ Detectado BEQ\n");
                    inst.opcode = OP_BEQ;
                    break;
                case 0b0001: // BNE
                    printf("→ Detectado BNE\n");
                    inst.opcode = OP_BNE;
                    break;
                case 0b1100: // BGT
                    printf("→ Detectado BGT\n");
                    inst.opcode = OP_BGT;
                    break;
                case 0b1011: // BLT
                    printf("→ Detectado BLT\n");
                    inst.opcode = OP_BLT;
                    break;
                case 0b1010: // BGE
                    printf("→ Detectado BGE\n");
                    inst.opcode = OP_BGE;
                    break;
                case 0b1101: // BLE
                    printf("→ Detectado BLE\n");
                    inst.opcode = OP_BLE;
                    break;
                default:
                    printf("⚠️ Condicional no soportado (cond = %d)\n", cond);
                    inst.opcode = OP_UNKNOWN;
                    break;
            }
            return inst;
        }


            

    uint32_t rd = hex & 0x1F;

    printf("Decoding 0x%08x → bits[31:21] = 0x%x\n", hex, opcode);

    switch (opcode) {
        case 0x588: // ADDS (inmediato)
            printf("→ Detectado ADDS (inmediato)\n");
            inst.opcode = OP_ADDS;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = (hex >> 10) & 0xFFF;
            inst.uses_imm = 1;
            break;

        case 0x558: // ADDS (registro)
            printf("→ Detectado ADDS (registro)\n");
            inst.opcode = OP_ADDS;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.rm = (hex >> 16) & 0x1F;
            inst.uses_imm = 0;
            break;

        case 0x758: { // SUBS / CMP (shift = 00 o 01)
            uint32_t shift = (hex >> 22) & 0x3;

            if (shift == 0b00) {
                if (rd == 31) {
                    inst.opcode = OP_CMP;
                    inst.rn = (hex >> 5) & 0x1F;
                    inst.rm = (hex >> 16) & 0x1F;
                    inst.uses_imm = 0;
                    printf("→ Detectado CMP (registro)\n");
                } else {
                    inst.opcode = OP_SUBS;
                    inst.rd = rd;
                    inst.rn = (hex >> 5) & 0x1F;
                    inst.rm = (hex >> 16) & 0x1F;
                    inst.uses_imm = 0;
                    printf("→ Detectado SUBS (registro)\n");
                }
            } else {
                if (rd == 31) {
                    inst.opcode = OP_CMP;
                    inst.rn = (hex >> 5) & 0x1F;
                    inst.imm = (hex >> 10) & 0xFFF;
                    if (shift == 0b01) {
                        inst.imm <<= 12;
                        printf("→ Shift aplicado: imm <<= 12 → %d\n", inst.imm);
                    }
                    inst.uses_imm = 1;
                    printf("→ Detectado CMP (inmediato)\n");
                } else {
                    inst.opcode = OP_SUBS;
                    inst.rd = rd;
                    inst.rn = (hex >> 5) & 0x1F;
                    inst.imm = (hex >> 10) & 0xFFF;
                    if (shift == 0b01) {
                        inst.imm <<= 12;
                        printf("→ Shift aplicado: imm <<= 12 → %d\n", inst.imm);
                    }
                    inst.uses_imm = 1;
                    printf("→ Detectado SUBS (inmediato)\n");
                }
            }
            break;
        }

        case 0x788: {
            uint32_t shift = (hex >> 22) & 0x1;
            if (rd == 31) {
                inst.opcode = OP_CMP;
                inst.rn = (hex >> 5) & 0x1F;
                inst.imm = (hex >> 10) & 0xFFF;
                if (shift) {
                    inst.imm <<= 12;
                    printf("→ Shift aplicado: imm <<= 12 → %d\n", inst.imm);
                }
                inst.uses_imm = 1;
                printf("→ Detectado CMP (inmediato%s): X%d - #%d\n",
                       shift ? " con shift" : "", inst.rn, inst.imm);
            } else {
                inst.opcode = OP_SUBS;
                inst.rd = rd;
                inst.rn = (hex >> 5) & 0x1F;
                inst.imm = (hex >> 10) & 0xFFF;
                if (shift) {
                    inst.imm <<= 12;
                    printf("→ Shift aplicado: imm <<= 12 → %d\n", inst.imm);
                }
                inst.uses_imm = 1;
                printf("→ Detectado SUBS (inmediato%s): X%d = X%d - #%d\n",
                       shift ? " con shift" : "", inst.rd, inst.rn, inst.imm);
            }
            break;
        }
        
        case 0x750: // ANDS (registro)
            printf("→ Detectado ANDS (registro)\n");
            inst.opcode = OP_ANDS;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.rm = (hex >> 16) & 0x1F;
            inst.uses_imm = 0;
            break;

        case 0x650: // EOR (registro)
            printf("→ Detectado EOR (registro)\n");
            inst.opcode = OP_EOR;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.rm = (hex >> 16) & 0x1F;
            inst.uses_imm = 0;
            break;

        case 0x550: // ORR (registro)
            printf("→ Detectado ORR (registro)\n");
            inst.opcode = OP_ORR;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.rm = (hex >> 16) & 0x1F;
            inst.uses_imm = 0;
            break;

        case 0x7c0: // STUR (registro)
            printf("→ Detectado STUR\n");
            inst.opcode = OP_STUR;
            inst.rt = hex & 0x1F;              // Registro a guardar (Xn)
            inst.rn = (hex >> 5) & 0x1F;       // Dirección base (Xn)
            inst.imm = (hex >> 12) & 0x1FF;    // Offset inmediato (9 bits)
            inst.uses_imm = 1;
            break;
        
        case 0x1c0: // STURB
            printf("→ Detectado STURB\n");
            inst.opcode = OP_STURB;
            inst.rt = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = ((hex >> 12) & 0x1FF); // offset de 9 bits
            break;
        
        case 0x3c0: // STURH
            printf("→ Detectado STURH\n");
            inst.opcode = OP_STURH;
            inst.rt = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = (hex >> 12) & 0x1FF; // offset inmediato de 9 bits
            break;
        
        case 0x7C2: // LDUR (registro)
            printf("→ Detectado LDUR\n");
            inst.opcode = OP_LDUR;
            inst.rt = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = (hex >> 12) & 0x1FF; // offset inmediato (9 bits)
            inst.uses_imm = 1;
            break;
        
        case 0x3c2: // LDURH
            printf("→ Detectado LDURH\n");
            inst.opcode = OP_LDURH;
            inst.rt = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = (hex >> 12) & 0x1FF; // offset inmediato (9 bits)
            inst.uses_imm = 1;
            break;
        
        case 0x1c2: // LDURB
            printf("→ Detectado LDURB\n");
            inst.opcode = OP_LDURB;
            inst.rt = hex & 0x1F;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = ((hex >> 12) & 0x1FF); // offset inmediato de 9 bits
            break;

        case 0x488: // ADD (inmediato, shift = 0)
        case 0x48a: { // ADD (inmediato, shift = 1)
                printf("→ Detectado ADD (inmediato)\n");
                inst.opcode = OP_ADD;
                inst.rd = rd;
                inst.rn = (hex >> 5) & 0x1F;
                inst.imm = (hex >> 10) & 0xFFF;
                inst.shift = (hex >> 22) & 0x1;  // shift: 0 o 1
                inst.uses_imm = 1;
            
                if (inst.shift == 1) {
                    printf("→ Shift aplicado: imm <<= 12 → %d\n", inst.imm << 12);
                }
            
                break;
            }
            
            case 0x458: // ADD (registro)
                printf("→ Detectado ADD (registro)\n");
                inst.opcode = OP_ADD;
                inst.rd = rd;
                inst.rn = (hex >> 5) & 0x1F;
                inst.rm = (hex >> 16) & 0x1F;
                inst.uses_imm = 0;
                break;

            case 0x4d8: // MUL (registro)
                printf("→ Detectado MUL\n");
                inst.opcode = OP_MUL;
                inst.rd = hex & 0x1F;
                inst.rn = (hex >> 5) & 0x1F;
                inst.rm = (hex >> 16) & 0x1F;
                inst.uses_imm = 0;
                break;
            
            case 0x6B0: { // BR (unconditional register branch)
                    inst.opcode = OP_BR;
                    inst.rn = (hex >> 5) & 0x1F; // Xn (registro con dirección)
                    printf("→ Detectado BR: salto a dirección contenida en X%d\n", inst.rn);
                    break;
                }
                
            
        default:
            inst.opcode = OP_UNKNOWN;
            printf("⚠️ Instrucción desconocida: 0x%08x (opcode: 0x%x)\n", hex, opcode);
            break;
    }

    return inst;
}

