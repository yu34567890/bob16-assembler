#include "lex.h"
#include "token.h"
#include "utils.h"
#include "instruction.h"

void print_instruction(const instruction_t *ins)
{
    const char *names[] =
    {
        [INS_EOP]     = "eop",
        [INVALID_INS] = "invalid",

        [INS_NOP] = "nop",

        [INS_ADD_RR]  = "add",
        [INS_ADD_RI4] = "add",
        [INS_ADD_RR2] = "add",
        [INS_ADD_RI7] = "add",

        [INS_AND_RR]  = "and",
        [INS_AND_RI4] = "and",
        [INS_AND_RR2] = "and",
        [INS_AND_RI7] = "and",

        [INS_NOT_RR]  = "not",
        [INS_NOT_RI4] = "not",

        [INS_LD]  = "ld",
        [INS_LDI] = "ldi",
        [INS_LDR] = "ldr",

        [INS_ST]  = "st",
        [INS_STI] = "sti",
        [INS_STR] = "str",

        [INS_BR] = "br",

        [INS_JMP_REG] = "jmp",

        [INS_JSR_REL] = "jsr",
        [INS_JSR_REG] = "jsr",

        [INS_LEA] = "lea",

        [INS_RET] = "ret",

        [INS_TRAP] = "trap",

        [INS_SUB]  = "sub",
        [INS_MUL]  = "mul",
        [INS_IDIV] = "idiv",
        [INS_MOD]  = "mod",
        [INS_INC]  = "inc",
        [INS_DEC]  = "dec",
        [INS_MIN]  = "min",
        [INS_MAX]  = "max",
        [INS_CMP]  = "cmp",

        [LABEL] = "label"
    };

    if (ins == NULL)
    {
        printf("<null instruction>\n");
        return;
    }

    if (ins->instruction_type == LABEL)
    {
        printf("label %s:\n", ins->label ? ins->label : "<null>");
        return;
    }

    printf("%s", names[ins->instruction_type]);

    switch (ins->instruction_type)
    {
        case INS_NOP:
        case INS_RET:
        case INS_EOP:
            break;

        case INS_ADD_RR:
        case INS_AND_RR:
            printf(" r%u, r%u, r%u", ins->dst, ins->src1, ins->src2);
            break;

        case INS_ADD_RI4:
        case INS_AND_RI4:
            printf(" r%u, r%u, #%d", ins->dst, ins->src1, ins->imm);
            break;

        case INS_ADD_RR2:
        case INS_AND_RR2:
            printf(" r%u, r%u", ins->dst, ins->src1);
            break;

        case INS_ADD_RI7:
        case INS_AND_RI7:
            printf(" r%u, #%d", ins->dst, ins->imm);
            break;

        case INS_NOT_RR:
            printf(" r%u, r%u", ins->dst, ins->src1);
            break;

        case INS_NOT_RI4:
            printf(" r%u, #%d", ins->dst, ins->imm);
            break;

        case INS_LD:
        case INS_LEA:
            printf(" r%u, #%d", ins->dst, ins->imm);
            break;

        case INS_LDI:
            printf(" r%u, #%d", ins->dst, ins->imm);
            break;

        case INS_LDR:
            printf(" r%u, r%u, #%d", ins->dst, ins->src1, ins->imm);
            break;

        case INS_ST:
        case INS_STI:
            printf(" r%u, #%d", ins->src1, ins->imm);
            break;

        case INS_STR:
            printf(" r%u, r%u, #%d", ins->src1, ins->src2, ins->imm);
            break;

        case INS_BR:
        	if (ins->label!=NULL)
        	{
        		printf(" %d, %s", ins-> src1, ins->label);
        		break;	
        	}

        	
            printf(" %d, #%d", ins->src1, ins->imm);
            break;

        case INS_JMP_REG:
        case INS_JSR_REG:
            printf(" r%u", ins->src1);
            break;

        case INS_JSR_REL:
            printf(" #%d", ins->imm);
            break;

        case INS_TRAP:
            printf(" #%d", ins->imm);
            break;

		case INS_CMP:
			printf(" r%u, r%u", ins->src1, ins->src2);
			break;

        case INS_SUB:
        case INS_MUL:
        case INS_IDIV:
        case INS_MOD:
            printf(" r%u, r%u, r%u", ins->dst ,ins->src1, ins->src2);
            break;

        case INS_INC:
        case INS_DEC:
            printf(" r%u", ins->dst);
            break;

        case INS_MIN:
        case INS_MAX:
            printf(" r%u, r%u, r%u", ins->dst, ins->src1, ins->src2);
            break;

        case INVALID_INS:
            break;

        default:
            printf(" <unknown>");
            break;
    }

    printf("\n");
}

int main()
{
    Token_t *tokens = tokenize("label:\ncmp r1 r2 \n;br 0b111 label");
    instruction_t *instructions = parse(tokens);
   	for (int i=0; instructions[i].instruction_type != INS_EOP; i++)
   	{
   		print_instruction(instructions + i);
   	}
}
