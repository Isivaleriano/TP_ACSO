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

###################EXECUTE#####################
#include "execute.h"
#include "shell.h"
#include <stdio.h>

void execute_instruction(Instruction inst) {
    switch (inst.opcode) {
        case OP_ADDS:
            if (inst.uses_imm) {
                int64_t result = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                NEXT_STATE.REGS[inst.rd] = result;
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado ADDS (imm): X%d = X%d + #%d = %ld\n",
                       inst.rd, inst.rn, inst.imm, result);
            } else {
                int64_t result = CURRENT_STATE.REGS[inst.rn] + CURRENT_STATE.REGS[inst.rm];
                NEXT_STATE.REGS[inst.rd] = result;
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado ADDS (reg): X%d = X%d + X%d = %ld\n",
                       inst.rd, inst.rn, inst.rm, result);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case OP_SUBS:
            if (inst.uses_imm) {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - inst.imm;
        
                if (inst.rd != 31) {
                    NEXT_STATE.REGS[inst.rd] = result;
                }
        
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
        
                printf("→ Ejecutado %s (imm): X%d = X%d - #%d = %ld\n",
                       inst.rd == 31 ? "CMP" : "SUBS",
                       inst.rd, inst.rn, inst.imm, result);
            } else {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - CURRENT_STATE.REGS[inst.rm];
        
                if (inst.rd != 31) {
                    NEXT_STATE.REGS[inst.rd] = result;
                }
        
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
        
                printf("→ Ejecutado %s (reg): X%d = X%d - X%d = %ld\n",
                       inst.rd == 31 ? "CMP" : "SUBS",
                       inst.rd, inst.rn, inst.rm, result);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        
        case OP_CMP:
            if (inst.uses_imm) {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - inst.imm;
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado CMP (imm): X%d - #%d = %ld\n",
                       inst.rn, inst.imm, result);
            } else {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - CURRENT_STATE.REGS[inst.rm];
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado CMP (reg): X%d - X%d = %ld\n",
                       inst.rn, inst.rm, result);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case OP_ANDS: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] & CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
            printf("→ Ejecutado ANDS: X%d = X%d & X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_EOR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] ^ CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            printf("→ Ejecutado EOR: X%d = X%d ^ X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_ORR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] | CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            printf("→ Ejecutado ORR: X%d = X%d | X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_HLT:
            printf("→ Ejecutado HLT: deteniendo simulación\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        
        case OP_B:
            NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            printf("→ Ejecutado B: salto a PC + %d = 0x%lx\n",
                    inst.branch_offset, NEXT_STATE.PC);
            break;
         
        case OP_MOVZ:
            NEXT_STATE.REGS[inst.rd] = ((uint64_t)inst.imm) << (16 * inst.hw);
            printf("→ Ejecutado MOVZ: X%d = #%d << %d = 0x%lx\n",
                   inst.rd, inst.imm, 16 * inst.hw, NEXT_STATE.REGS[inst.rd]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        
        case OP_BEQ:
            if (CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BEQ: salto a PC + %d = 0x%lx (FLAG_Z = 1)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BEQ: no salta (FLAG_Z = 0), PC = 0x%lx\n",
                       NEXT_STATE.PC);
            }
            break;
        case OP_BNE:
            if (!CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BNE: salto a PC + %d = 0x%lx (FLAG_Z = 0)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BNE: no salta (FLAG_Z = 1), PC = 0x%lx\n",
                       NEXT_STATE.PC);
            }
            break;

        case OP_BGT:
            if (!CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BGT: salto a PC + %d = 0x%lx (Z=0, N=0)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BGT: no salta (Z=%d, N=%d)\n",
                       CURRENT_STATE.FLAG_Z, CURRENT_STATE.FLAG_N);
            }
            break;
        
        case OP_BLT:
            if (CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BLT: salto a PC + %d = 0x%lx (N=1)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BLT: no salta (N=%d)\n", CURRENT_STATE.FLAG_N);
            }
            break;

        case OP_BGE:
            if (!CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BGE: salto a PC + %d = 0x%lx (N=0)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BGE: no salta (N=%d)\n", CURRENT_STATE.FLAG_N);
            }
            break;

        case OP_BLE:
            if (CURRENT_STATE.FLAG_Z || CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BLE: salto a PC + %d = 0x%lx (Z=%d, N=%d)\n",
                       inst.branch_offset, NEXT_STATE.PC,
                       CURRENT_STATE.FLAG_Z, CURRENT_STATE.FLAG_N);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BLE: no salta (Z=%d, N=%d)\n",
                       CURRENT_STATE.FLAG_Z, CURRENT_STATE.FLAG_N);
            }
            break;

        case OP_LSL:
            NEXT_STATE.REGS[inst.rd] = CURRENT_STATE.REGS[inst.rn] << inst.shift;
            printf("→ Ejecutado LSL: X%d = X%d << #%d = 0x%lx\n",
                   inst.rd, inst.rn, inst.shift, NEXT_STATE.REGS[inst.rd]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;  
              
        case OP_LSR:
            NEXT_STATE.REGS[inst.rd] = ((uint64_t)CURRENT_STATE.REGS[inst.rn]) >> inst.shift;
            printf("→ Ejecutado LSR: X%d = X%d >> #%d = 0x%lx\n",
                   inst.rd, inst.rn, inst.shift, NEXT_STATE.REGS[inst.rd]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

        case OP_STUR: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                mem_write_32(addr, CURRENT_STATE.REGS[inst.rt] & 0xFFFFFFFF);
                printf("→ Ejecutado STUR: M[0x%lx] = X%d = 0x%lx\n",
                       addr, inst.rt, CURRENT_STATE.REGS[inst.rt]);
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }  
        
        case OP_STURB: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                uint32_t byte = CURRENT_STATE.REGS[inst.rt] & 0xFF;  // Tomamos solo los últimos 8 bits
            
                // Guardamos como si fuera un entero de 32 bits, pero solo usamos los 8 bits bajos.
                mem_write_32(addr, byte);
            
                printf("→ Ejecutado STURB: M[0x%lx] = X%d(7:0) = 0x%x\n", addr, inst.rt, byte);
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }
        
        case OP_STURH: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                uint16_t value = CURRENT_STATE.REGS[inst.rt] & 0xFFFF;
            
                // Simulamos escritura de 16 bits usando mem_write_32,
                // preservando los otros 16 bits (emulando STURH)
                uint32_t old = mem_read_32(addr & ~0x3); // leer palabra alineada
                uint32_t shift = (addr & 0x3) * 8;
                uint32_t mask = 0xFFFF << shift;
                uint32_t new_value = (old & ~mask) | (value << shift);
                mem_write_32(addr & ~0x3, new_value);
            
                printf("→ Ejecutado STURH: M[0x%lx] = 0x%x (16 bits)\n", addr, value);
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }

        case OP_LDUR: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                
                // Leemos dos palabras de 32 bits y las combinamos en 64
                uint32_t lower = mem_read_32(addr);
                uint32_t upper = mem_read_32(addr + 4);
            
                NEXT_STATE.REGS[inst.rt] = ((uint64_t)upper << 32) | lower;
            
                printf("→ Ejecutado LDUR: X%d = M[X%d + #%d] = 0x%lx\n",
                       inst.rt, inst.rn, inst.imm, NEXT_STATE.REGS[inst.rt]);
            
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }
        case OP_LDURH: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            
                // Leer 32 bits y quedarnos solo con los 16 bits bajos
                uint32_t data = mem_read_32(addr);
                uint16_t halfword = data & 0xFFFF;
            
                // Guardar en X[rt] con zero-extension (automática al asignar a uint64_t)
                NEXT_STATE.REGS[inst.rt] = (uint64_t)halfword;
            
                printf("→ Ejecutado LDURH: X%d = M[X%d + #%d](16 bits) = 0x%04x\n",
                       inst.rt, inst.rn, inst.imm, halfword);
            
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }
                
        case OP_LDURB: {
                uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
                uint32_t word = mem_read_32(addr);
                uint8_t byte = word & 0xFF;
                NEXT_STATE.REGS[inst.rt] = byte;
                printf("→ Ejecutado LDURB: X%d = M[X%d + #%d] & 0xFF → 0x%02x\n",
                       inst.rt, inst.rn, inst.imm, byte);
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                break;
            }

        case OP_ADD:
            if (inst.uses_imm) {
                int64_t imm_val = inst.imm;
                if (inst.shift) imm_val <<= 12;
                NEXT_STATE.REGS[inst.rd] = CURRENT_STATE.REGS[inst.rn] + imm_val;
                printf("→ Ejecutado ADD (inmediato): X%d = X%d + #%ld = %ld\n",
                    inst.rd, inst.rn, imm_val, NEXT_STATE.REGS[inst.rd]);
            } else {
                NEXT_STATE.REGS[inst.rd] = CURRENT_STATE.REGS[inst.rn] + CURRENT_STATE.REGS[inst.rm];
                printf("→ Ejecutado ADD (registro): X%d = X%d + X%d = %ld\n",
                    inst.rd, inst.rn, inst.rm, NEXT_STATE.REGS[inst.rd]);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        
        case OP_MUL:
            NEXT_STATE.REGS[inst.rd] = CURRENT_STATE.REGS[inst.rn] * CURRENT_STATE.REGS[inst.rm];
            printf("→ Ejecutado MUL: X%d = X%d * X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, NEXT_STATE.REGS[inst.rd]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;

            case OP_CBZ:
            if (CURRENT_STATE.REGS[inst.rt] == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado CBZ: X%d es 0 → salto a PC = 0x%lx\n", inst.rt, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado CBZ: X%d no es 0 → no salta\n", inst.rt);
            }
            break;
        
        case OP_CBNZ:
            if (CURRENT_STATE.REGS[inst.rt] != 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado CBNZ: X%d != 0 → salto a PC = 0x%lx\n", inst.rt, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado CBNZ: X%d == 0 → no salto\n", inst.rt);
            }
            break;
        
        case OP_BR:
            NEXT_STATE.PC = CURRENT_STATE.REGS[inst.rn];
            printf("→ Ejecutado BR: salto a dirección contenida en X%d = 0x%lx\n",
                   inst.rn, NEXT_STATE.PC);
            break;

        default:
            printf("⚠️ Instrucción no soportada (OP_UNKNOWN)\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
    }
}

