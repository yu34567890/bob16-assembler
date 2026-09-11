#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "token.h"


typedef enum INSTRUCTION_E
{
	/* error handling */
	INS_EOP, // end of program > end of file
	INVALID_INS, 

    /* basic */
    INS_NOP,

    /* ADD */
    INS_ADD_RR,      // ADD Rd, Rs1, Rs2
    INS_ADD_RI4,     // ADD Rd, Rs, imm4
    INS_ADD_RR2,     // ADD Rd, Rs1
    INS_ADD_RI7,     // ADD Rd, imm7

    /* AND */
    INS_AND_RR,
    INS_AND_RI4,
    INS_AND_RR2,
    INS_AND_RI7,

    /* NOT */
    INS_NOT_RR,      // NOT Rd, Rs
    INS_NOT_RI4,     // NOT Rd, imm4

    /* memory */
    INS_LD,          // LD Rd, PC + imm9
    INS_LDI,         // LDI Rd, mem[mem[PC + imm9]]
    INS_LDR,         // LDR Rd, Rb + imm6

    INS_ST,          // ST Rs, PC + imm9
    INS_STI,         // STI Rs, mem[PC + imm9]
    INS_STR,         // STR Rs, Rb + imm6

    /* branches */
    INS_BR,          // BR condition, imm9

    /* jump */
    INS_JMP_REG,     // JMP Rn

    /* subroutine */
    INS_JSR_REL,     // JSR imm11
    INS_JSR_REG,     // JSR Rn

    /* address */
    INS_LEA,          // LEA Rd, PC + imm9

    /* return */
    INS_RET,

    /* traps */
    INS_TRAP,

    /* arithmetic expansion */
    INS_SUB,
    INS_MUL,
    INS_IDIV,
    INS_MOD,
    INS_INC,
    INS_DEC,
    INS_MIN,
    INS_MAX,
    INS_CMP,

    /* doesnt exist or parser helper */
    LABEL,
    
    

} instruction_type_t;

typedef struct instruction_struct 
{
    instruction_type_t instruction_type;
    uint8_t src1;
    uint8_t src2;
    uint8_t dst;
    int16_t imm;
    char *label;
    
} instruction_t;

instruction_t *parse(Token_t *tokens);

#endif
