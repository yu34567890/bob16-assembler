#ifndef INSTRUCTION_H
#define INSTRUCTION_H
#include <stdint.h>

typedef struct instruction_struct 
{
    uint8_t instruction_type;
    uint8_t src1;
    uint8_t src2;
    uint8_t dst;
    int16_t imm;
    
} instruction_t;


#endif