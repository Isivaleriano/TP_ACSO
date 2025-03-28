#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include "decode.h"
#include "execute.h"


void process_instruction() {
    uint32_t instruction_hex = mem_read_32(CURRENT_STATE.PC);
    Instruction inst = decode_instruction(instruction_hex);
    execute_instruction(inst);
    // NO actualices el PC acá: ya lo hace execute_instruction si es necesario
}