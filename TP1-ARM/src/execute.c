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
            } else {
                result = CURRENT_STATE.REGS[inst.rn] + CURRENT_STATE.REGS[inst.rm];
            }
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_MUL: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] * CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
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
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_CMP: {
            int64_t operand2 = inst.uses_imm ? inst.imm : CURRENT_STATE.REGS[inst.rm];
            int64_t result = CURRENT_STATE.REGS[inst.rn] - operand2;
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Operaciones lógicas
        case OP_ANDS: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] & CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_EOR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] ^ CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_ORR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] | CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Shift y movimientos
        case OP_MOVZ: {
            uint64_t result = ((uint64_t)inst.imm) << (16 * inst.hw);
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LSL: {
            uint64_t result = CURRENT_STATE.REGS[inst.rn] << inst.shift;
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LSR: {
            uint64_t result = ((uint64_t)CURRENT_STATE.REGS[inst.rn]) >> inst.shift;
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Branches
        case OP_B: {
            NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            break;
        }

        case OP_BR: {
            NEXT_STATE.PC = CURRENT_STATE.REGS[inst.rn];
            break;
        }

        case OP_BEQ: {
            if (CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_BNE: {
            if (!CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_BGT: {
            if (!CURRENT_STATE.FLAG_Z && !CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_BLT: {
            if (CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_BGE: {
            if (!CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_BLE: {
            if (CURRENT_STATE.FLAG_Z || CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_CBZ: {
            if (CURRENT_STATE.REGS[inst.rt] == 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            }
            break;
        }

        case OP_CBNZ: {
            if (CURRENT_STATE.REGS[inst.rt] != 0) {
                NEXT_STATE.PC = CURRENT_STATE.PC + inst.branch_offset;
            } else {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
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
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LDURH: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t data = mem_read_32(addr);
            uint16_t result = data & 0xFFFF;
        
            NEXT_STATE.REGS[inst.rt] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_LDURB: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t data = mem_read_32(addr);
            uint8_t result = data & 0xFF;
        
            NEXT_STATE.REGS[inst.rt] = result;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_STUR: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint32_t value = CURRENT_STATE.REGS[inst.rt] & 0xFFFFFFFF;
        
            mem_write_32(addr, value);
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
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        case OP_STURB: {
            uint64_t addr = CURRENT_STATE.REGS[inst.rn] + inst.imm;
            uint8_t value = CURRENT_STATE.REGS[inst.rt] & 0xFF;
        
            mem_write_32(addr, value);
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        // Instrucciones de control
        case OP_HLT: {
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }

        default: {
            RUN_BIT = 0;
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            break;
        }


    }
}

