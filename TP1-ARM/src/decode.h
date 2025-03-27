#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

typedef enum {
    OP_ADDS,
    OP_UNKNOWN,
    OP_SUBS,
    OP_CMP,
    OP_ANDS,
    OP_EOR,
    OP_ORR,
    OP_B,
    OP_HLT
} Opcode;

typedef struct {
    Opcode opcode;
    int rd;
    int rn;
    int rm;
    int imm;
    int uses_imm;
    int branch_offset;
    uint32_t raw;
} Instruction;

Instruction decode_instruction(uint32_t instruction_hex);

#endif
