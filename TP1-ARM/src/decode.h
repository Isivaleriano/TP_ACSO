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
    OP_BR,
    OP_BEQ,
    OP_BNE,
    OP_BGT,
    OP_BLT,
    OP_BLE,
    OP_BGE,
    OP_LSL,
    OP_LSR,
    OP_STUR,
    OP_STURB,
    OP_STURH,
    OP_LDUR,
    OP_LDURH,
    OP_LDURB,
    OP_MOVZ,
    OP_ADD,
    OP_MUL,
    OP_CBZ,
    OP_CBNZ,
    OP_HLT
} Opcode;

typedef struct {
    Opcode opcode;
    int rd, rn, rm, rt;
    int imm;
    int uses_imm;
    int branch_offset;
    int hw; 
    int shift; 
    int offset;
    uint32_t raw;
} Instruction;

Instruction decode_instruction(uint32_t instruction_hex);

#endif
