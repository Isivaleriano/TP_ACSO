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
        
        

        default:
            printf("⚠️ Instrucción no soportada (OP_UNKNOWN)\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
    }
}

