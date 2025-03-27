#include "decode.h"
#include <stdio.h>

Instruction decode_instruction(uint32_t hex) {
    Instruction inst = {0};
    inst.raw = hex;

    // Detectar HLT directamente
    if (hex == 0xD4400000) {
        inst.opcode = OP_HLT;
        printf("→ Detectado HLT\n");
        return inst;
    }

    uint32_t opcode = (hex >> 21) & 0x7FF;
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

        case 0x788: // SUBS (inmediato con shift == 01)
            inst.opcode = OP_SUBS;
            inst.rd = rd;
            inst.rn = (hex >> 5) & 0x1F;
            inst.imm = (hex >> 10) & 0xFFF;
            inst.imm <<= 12;
            inst.uses_imm = 1;
            printf("→ Detectado SUBS (inmediato con shift): X%d = X%d - #%d\n",
                   inst.rd, inst.rn, inst.imm);
            break;

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

        default:
            inst.opcode = OP_UNKNOWN;
            printf("⚠️ Instrucción desconocida: 0x%08x (opcode: 0x%x)\n", hex, opcode);
            break;
    }

    return inst;
}
