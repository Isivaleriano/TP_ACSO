#include "execute.h"
#include "shell.h"
#include <stdio.h>

void execute_instruction(Instruction inst) {
    switch (inst.opcode) {
        // Aritmética sin flags
        case OP_ADD: {
            int64_t result;
            if (inst.uses_imm) {
                int64_t imm_val = inst.imm;
                if (inst.shift) imm_val <<= 12;
                result = CURRENT_STATE.REGS[inst.rn] + imm_val;
                printf("→ Ejecutado ADD (inmediato): X%d = X%d + #%ld = %ld\n",
                       inst.rd, inst.rn, imm_val, result);
            } else {
                result = CURRENT_STATE.REGS[inst.rn] + CURRENT_STATE.REGS[inst.rm];
                printf("→ Ejecutado ADD (registro): X%d = X%d + X%d = %ld\n",
                       inst.rd, inst.rn, inst.rm, result);
            }
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_MUL: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] * CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            printf("→ Ejecutado MUL: X%d = X%d * X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Aritmética con flags
        case OP_ADDS: {
            int64_t operand2 = inst.uses_imm ? inst.imm : CURRENT_STATE.REGS[inst.rm];
            int64_t result = CURRENT_STATE.REGS[inst.rn] + operand2;
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
        
            if (inst.uses_imm) {
                printf("→ Ejecutado ADDS (imm): X%d = X%d + #%d = %ld\n",
                       inst.rd, inst.rn, inst.imm, result);
            } else {
                printf("→ Ejecutado ADDS (reg): X%d = X%d + X%d = %ld\n",
                       inst.rd, inst.rn, inst.rm, result);
            }
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_SUBS: {
            int64_t operand2 = inst.uses_imm ? inst.imm : CURRENT_STATE.REGS[inst.rm];
            int64_t result = CURRENT_STATE.REGS[inst.rn] - operand2;
        
            // Guardamos el resultado si no es CMP
            if (inst.rd != 31) {
                NEXT_STATE.REGS[inst.rd] = result;
            }
        
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
        
            if (inst.uses_imm) {
                printf("→ Ejecutado %s (imm): X%d = X%d - #%d = %ld\n",
                       inst.rd == 31 ? "CMP" : "SUBS",
                       inst.rd, inst.rn, inst.imm, result);
            } else {
                printf("→ Ejecutado %s (reg): X%d = X%d - X%d = %ld\n",
                       inst.rd == 31 ? "CMP" : "SUBS",
                       inst.rd, inst.rn, inst.rm, result);
            }
        
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_CMP: {
            int64_t operand2 = inst.uses_imm ? inst.imm : CURRENT_STATE.REGS[inst.rm];
            int64_t result = CURRENT_STATE.REGS[inst.rn] - operand2;
        
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
        
            if (inst.uses_imm) {
                printf("→ Ejecutado CMP (imm): X%d - #%d = %ld\n",
                       inst.rn, inst.imm, result);
            } else {
                printf("→ Ejecutado CMP (reg): X%d - X%d = %ld\n",
                       inst.rn, inst.rm, result);
            }
        
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Operaciones lógicas
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

        // Shift y movimientos
        case OP_MOVZ: {
            uint64_t result = ((uint64_t)inst.imm) << (16 * inst.hw);
            NEXT_STATE.REGS[inst.rd] = result;
        
            printf("→ Ejecutado MOVZ: X%d = #%d << %d = 0x%lx\n",
                   inst.rd, inst.imm, 16 * inst.hw, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LSL: {
            uint64_t result = CURRENT_STATE.REGS[inst.rn] << inst.shift;
            NEXT_STATE.REGS[inst.rd] = result;
        
            printf("→ Ejecutado LSL: X%d = X%d << #%d = 0x%lx\n",
                   inst.rd, inst.rn, inst.shift, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LSR: {
            uint64_t result = ((uint64_t)CURRENT_STATE.REGS[inst.rn]) >> inst.shift;
            NEXT_STATE.REGS[inst.rd] = result;
        
            printf("→ Ejecutado LSR: X%d = X%d >> #%d = 0x%lx\n",
                   inst.rd, inst.rn, inst.shift, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Branches
        case OP_B: {
            NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            printf("→ Ejecutado B: salto a PC + %d = 0x%lx\n",
                   inst.branch_offset, NEXT_STATE.PC);
            break;
        }

        case OP_BR: {
            NEXT_STATE.PC = CURRENT_STATE.REGS[inst.rn];
            printf("→ Ejecutado BR: salto a dirección contenida en X%d = 0x%lx\n",
                   inst.rn, NEXT_STATE.PC);
            break;
        }

        case OP_BEQ: {
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
        }

        case OP_BNE: {
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
        }

        case OP_BGT: {
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
        }

        case OP_BLT: {
            if (CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BLT: salto a PC + %d = 0x%lx (N=1)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BLT: no salta (N=%d)\n", CURRENT_STATE.FLAG_N);
            }
            break;
        }

        case OP_BGE: {
            if (!CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado BGE: salto a PC + %d = 0x%lx (N=0)\n",
                       inst.branch_offset, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado BGE: no salta (N=%d)\n", CURRENT_STATE.FLAG_N);
            }
            break;
        }

        case OP_BLE: {
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
        }

        case OP_CBZ: {
            if (CURRENT_STATE.REGS[inst.rt] == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado CBZ: X%d es 0 → salto a PC = 0x%lx\n",
                       inst.rt, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado CBZ: X%d no es 0 → no salta\n", inst.rt);
            }
            break;
        }

        case OP_CBNZ: {
            if (CURRENT_STATE.REGS[inst.rt] != 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
                printf("→ Ejecutado CBNZ: X%d != 0 → salto a PC = 0x%lx\n",
                       inst.rt, NEXT_STATE.PC);
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
                printf("→ Ejecutado CBNZ: X%d == 0 → no salto\n", inst.rt);
            }
            break;
        }

        // Instrucciones de memoria
        case OP_LDUR: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t lower = mem_read_32(addr);
            uint32_t upper = mem_read_32(addr + 4);
            uint64_t result = ((uint64_t)upper << 32) | lower;
        
            NEXT_STATE.REGS[inst.rt] = result;
            printf("→ Ejecutado LDUR: X%d = M[X%d + #%d] = 0x%lx\n",
                   inst.rt, inst.rn, inst.imm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LDURH: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t data = mem_read_32(addr);
            uint16_t result = data & 0xFFFF;
        
            NEXT_STATE.REGS[inst.rt] = result;
            printf("→ Ejecutado LDURH: X%d = M[X%d + #%d](16 bits) = 0x%04x\n",
                   inst.rt, inst.rn, inst.imm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LDURB: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t data = mem_read_32(addr);
            uint8_t result = data & 0xFF;
        
            NEXT_STATE.REGS[inst.rt] = result;
            printf("→ Ejecutado LDURB: X%d = M[X%d + #%d] & 0xFF → 0x%02x\n",
                   inst.rt, inst.rn, inst.imm, result);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_STUR: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t value = CURRENT_STATE.REGS[inst.rt] & 0xFFFFFFFF;
        
            mem_write_32(addr, value);
            printf("→ Ejecutado STUR: M[0x%lx] = X%d = 0x%lx\n",
                   addr, inst.rt, CURRENT_STATE.REGS[inst.rt]);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_STURH: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint16_t value = CURRENT_STATE.REGS[inst.rt] & 0xFFFF;
        
            uint32_t old = mem_read_32(addr & ~0x3); // Alinear a 4 bytes
            uint32_t shift = (addr & 0x3) * 8;
            uint32_t mask = 0xFFFF << shift;
            uint32_t new_value = (old & ~mask) | (value << shift);
        
            mem_write_32(addr & ~0x3, new_value);
            printf("→ Ejecutado STURH: M[0x%lx] = 0x%x (16 bits)\n", addr, value);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_STURB: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint8_t value = CURRENT_STATE.REGS[inst.rt] & 0xFF;
        
            mem_write_32(addr, value);
            printf("→ Ejecutado STURB: M[0x%lx] = X%d(7:0) = 0x%x\n",
                   addr, inst.rt, value);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Instrucciones de control
        case OP_HLT: {
            printf("→ Ejecutado HLT: deteniendo simulación\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        default: {
            printf("⚠️ Instrucción no soportada (OP_UNKNOWN)\n");
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }


    }
}

