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
            break;

        case OP_SUBS:
            if (inst.uses_imm) {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - inst.imm;
                NEXT_STATE.REGS[inst.rd] = result;
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado SUBS (imm): X%d = X%d - #%d = %ld\n",
                       inst.rd, inst.rn, inst.imm, result);
            } else {
                int64_t result = CURRENT_STATE.REGS[inst.rn] - CURRENT_STATE.REGS[inst.rm];
                NEXT_STATE.REGS[inst.rd] = result;
                NEXT_STATE.FLAG_Z = (result == 0);
                NEXT_STATE.FLAG_N = (result < 0);
                printf("→ Ejecutado SUBS (reg): X%d = X%d - X%d = %ld\n",
                       inst.rd, inst.rn, inst.rm, result);
            }
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
            break;

        case OP_ANDS: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] & CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            NEXT_STATE.FLAG_Z = (result == 0);
            NEXT_STATE.FLAG_N = (result < 0);
            printf("→ Ejecutado ANDS: X%d = X%d & X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            break;
        }

        case OP_EOR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] ^ CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            printf("→ Ejecutado EOR: X%d = X%d ^ X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            break;
        }

        case OP_ORR: {
            int64_t result = CURRENT_STATE.REGS[inst.rn] | CURRENT_STATE.REGS[inst.rm];
            NEXT_STATE.REGS[inst.rd] = result;
            printf("→ Ejecutado ORR: X%d = X%d | X%d = %ld\n",
                   inst.rd, inst.rn, inst.rm, result);
            break;
        }

        case OP_HLT:
            printf("→ Ejecutado HLT: deteniendo simulación\n");
            RUN_BIT = 0;
            break;

        default:
            printf("⚠️ Instrucción no soportada (OP_UNKNOWN)\n");
            RUN_BIT = 0;
            break;
    }
}
