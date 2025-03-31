#include "decode.h"
#include <stdio.h>

// ==== JUMPS ====

int decode_b(uint32_t hex, Instruction* inst) {
    if ((hex & 0x7C000000) == 0x14000000) {
        inst->opcode = OP_B;
        int32_t imm26 = hex & 0x03FFFFFF;
        if (imm26 & (1 << 25)) {
            imm26 |= ~0x03FFFFFF;
        }
        inst->branch_offset = imm26 << 2;
        return 1;
    }
    return 0;
}

int decode_br(uint32_t hex, uint32_t opcode, Instruction* inst) {
    if (opcode == 0x6B0) {
        inst->opcode = OP_BR;
        inst->rn = (hex >> 5) & 0x1F;
        return 1;
    }
    return 0;
}

int decode_cbz_cbnz(uint32_t hex, Instruction* inst) {
    if ((hex & 0x7E000000) == 0x34000000) {
        uint32_t rt = hex & 0x1F;
        int32_t offset = (hex >> 5) & 0x7FFFF;
        if (offset & (1 << 18)) offset |= ~0x7FFFF;
        offset <<= 2;

        inst->rt = rt;
        inst->branch_offset = offset;
        if ((hex & (1 << 24)) == 0) {
            inst->opcode = OP_CBZ;
        } else {
            inst->opcode = OP_CBNZ;
        }
        return 1;
    }
    return 0;
}

int decode_b_cond(uint32_t hex, Instruction* inst) {
    if ((hex & 0xFF000010) == 0x54000000) {
        uint8_t cond = hex & 0xF;
        int32_t offset = ((hex >> 5) & 0x7FFFF);
        if (offset & (1 << 18)) offset |= ~0x7FFFF;
        offset <<= 2;

        inst->branch_offset = offset;
        switch (cond) {
            case 0b0000: inst->opcode = OP_BEQ; break;
            case 0b0001: inst->opcode = OP_BNE; break;
            case 0b1100: inst->opcode = OP_BGT; break;
            case 0b1011: inst->opcode = OP_BLT; break;
            case 0b1010: inst->opcode = OP_BGE; break;
            case 0b1101: inst->opcode = OP_BLE; break;
            default:
                inst->opcode = OP_UNKNOWN;
                break;
        }
        return 1;
    }
    return 0;
}

// ==== ARITHMETIC ====

int decode_adds_subs_cmp(uint32_t hex, uint32_t opcode, Instruction* inst) {
    uint32_t rd = hex & 0x1F;
    inst->rd = rd;

    switch (opcode) {
        case 0x588:
            inst->opcode = OP_ADDS;
            inst->rn = (hex >> 5) & 0x1F;
            inst->imm = (hex >> 10) & 0xFFF;
            inst->uses_imm = 1;
            return 1;

        case 0x558:
            inst->opcode = OP_ADDS;
            inst->rn = (hex >> 5) & 0x1F;
            inst->rm = (hex >> 16) & 0x1F;
            inst->uses_imm = 0;
            return 1;

        case 0x758: {
            uint32_t shift = (hex >> 22) & 0x3;
            if (shift == 0b00) {
                inst->rn = (hex >> 5) & 0x1F;
                inst->rm = (hex >> 16) & 0x1F;
                inst->uses_imm = 0;
                if (rd == 31) {
                    inst->opcode = OP_CMP;
                } else {
                    inst->opcode = OP_SUBS;
                }
                return 1;
            }
            break;
        }

        case 0x788: {
            uint32_t shift = (hex >> 22) & 0x1;
            inst->imm = (hex >> 10) & 0xFFF;
            if (shift) {
                inst->imm <<= 12;
            }
            inst->uses_imm = 1;
            inst->rn = (hex >> 5) & 0x1F;
            if (rd == 31) {
                inst->opcode = OP_CMP;
            } else {
                inst->opcode = OP_SUBS;
            }
            return 1;
        }
    }
    return 0;
}

int decode_add_mul(uint32_t hex, uint32_t opcode, Instruction* inst) {
    inst->rd = hex & 0x1F;
    inst->rn = (hex >> 5) & 0x1F;

    switch (opcode) {
        case 0x488:
        case 0x48A:
            inst->opcode = OP_ADD;
            inst->imm = (hex >> 10) & 0xFFF;
            inst->shift = (hex >> 22) & 0x1;
            inst->uses_imm = 1;
            if (inst->shift == 1) {
            }
            return 1;

        case 0x458:
            inst->opcode = OP_ADD;
            inst->rm = (hex >> 16) & 0x1F;
            inst->uses_imm = 0;
            return 1;

        case 0x4D8:
            inst->opcode = OP_MUL;
            inst->rm = (hex >> 16) & 0x1F;
            inst->uses_imm = 0;
            return 1;
    }
    return 0;
}

// ==== LOGICAL ====

int decode_logical(uint32_t hex, uint32_t opcode, Instruction* inst) {
    uint32_t rd = hex & 0x1F;
    inst->rd = rd;
    inst->rn = (hex >> 5) & 0x1F;
    inst->rm = (hex >> 16) & 0x1F;
    inst->uses_imm = 0;

    switch (opcode) {
        case 0x750:
            inst->opcode = OP_ANDS;
            return 1;

        case 0x650:
            inst->opcode = OP_EOR;
            return 1;

        case 0x550:
            inst->opcode = OP_ORR;
            return 1;
    }
    return 0;
}

// ==== MEMORY ====

int decode_mem_access(uint32_t hex, uint32_t opcode, Instruction* inst) {
    inst->rt = hex & 0x1F;
    inst->rn = (hex >> 5) & 0x1F;
    inst->imm = (hex >> 12) & 0x1FF;

    switch (opcode) {
        case 0x7C0:
            inst->opcode = OP_STUR;
            inst->uses_imm = 1;
            return 1;

        case 0x1C0:
            inst->opcode = OP_STURB;
            return 1;

        case 0x3C0:
            inst->opcode = OP_STURH;
            return 1;

        case 0x7C2:
            inst->opcode = OP_LDUR;
            inst->uses_imm = 1;
            return 1;

        case 0x3C2:
            inst->opcode = OP_LDURH;
            inst->uses_imm = 1;
            return 1;

        case 0x1C2:
            inst->opcode = OP_LDURB;
            return 1;
    }
    return 0;
}

// ==== MISC ====

int decode_movz(uint32_t hex, uint32_t opcode, Instruction* inst) {
    if (opcode >= 0x694 && opcode <= 0x697) {
        inst->opcode = OP_MOVZ;
        inst->rd = hex & 0x1F;
        inst->imm = (hex >> 5) & 0xFFFF;
        inst->hw = (hex >> 21) & 0x3;
        inst->uses_imm = 1;
        return 1;
    }
    return 0;
}

// ==== MAIN DECODER ====

Instruction decode_instruction(uint32_t hex) {
    Instruction inst = {0};
    inst.raw = hex;

    if (hex == 0xD4400000) {
        inst.opcode = OP_HLT;
        return inst;
    }

    if (decode_shift(hex, &inst)) return inst;

    uint32_t opcode = (hex >> 21) & 0x7FF;

    if (decode_movz(hex, opcode, &inst)) return inst;
    if (decode_b(hex, &inst)) return inst;
    if (decode_br(hex, opcode, &inst)) return inst;
    if (decode_cbz_cbnz(hex, &inst)) return inst;
    if (decode_b_cond(hex, &inst)) return inst;
    if (decode_adds_subs_cmp(hex, opcode, &inst)) return inst;
    if (decode_add_mul(hex, opcode, &inst)) return inst;
    if (decode_logical(hex, opcode, &inst)) return inst;
    if (decode_mem_access(hex, opcode, &inst)) return inst;

    return inst;
}