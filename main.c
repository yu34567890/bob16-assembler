#include "machine_code.h"
#include "lex.h"
#include "token.h"
#include "utils.h"
#include "instruction.h"


void print_instruction(const instruction_t *ins)
{
	const char *names[] =
	{
		[INS_EOP]	 = "eop",
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
		[INS_NOT_R] = "not",

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

		[WORD] = "WORD",
		[ORG] = "ORG",

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

		case INS_NOT_R:
			printf(" r%u", ins->dst);
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
				printf(" %d, %s", ins->dst, ins->label);
				break;	
			}			
			printf(" %d, #%d", ins->dst, ins->imm);
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
			
		case ORG:
		case WORD:
				if (ins->label)
			{
				printf(" %s", ins->label);
			}
			else
				printf(" %X", ins->imm);
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



/*
int main(int argc, char **argv) {
	initthings(argc, argv);
	// The include marker at the top of the file
	if (IncludeFile)
	  DoInclude(IncludeFile, 0);
	IncludeFile = NULL;
	write_include_marker(C->out->f, 1, C->filename, "");
	ProcessContext();
	fclose(C->out->f);
	return EXIT_SUCCESS;
}
*/

#include "shamefull_ai_slop/minipp.h"

#define panic(msg) do { fputs(msg, stderr); exit(1); } while (0)

char *args_shift(int *argc, char ***argv) {
	char *out = (*argv)[0];
	--(*argc);
	++(*argv);
	return out;
}

typedef struct {
	char infile[4096];
	char outfile[4096];
} ArgData;

ArgData parse_args(int *argc, char ***argv) {
	ArgData out = {0};
	while (*argc > 0) {
		char *arg = args_shift(argc, argv);
		if (!strcmp(arg, "-o")) {
			if (argc == 0) panic("Fuck you");
			char *outfile = args_shift(argc, argv);
			snprintf(out.outfile, 4096, outfile);
	 	} else {
			// Assumed infile
			snprintf(out.infile, 4096, arg);
	 	}
	}

	return out;
}

int main(int argc, char**argv)
{
	ArgData args = parse_args(&argc, &argv);

	FILE *infile = fopen(args.infile, "rb");
	FILE *outfile = fopen(args.outfile, "wb");
	uint16_t outbuf[65536];

	if (infile == NULL)
	{
		perror("error opening file");
		exit(35);
	}
	if (*args.outfile != 0 && outfile == NULL) {
		perror("error opening file");
		exit(34);
	}

	fseek(infile, 0, SEEK_END);
	long filesize = ftell(infile);
	rewind(infile);

	char* buffer = malloc(filesize+1);
	size_t bytesRead = fread(buffer, 1, filesize, infile);

	buffer[bytesRead] = 0;

	/* AI CODE !!!!!*/
    minipp_result pp = minipp_process(buffer, args.infile);

    if (pp.ok) {
        fprintf(stderr, "preprocessor error: %s\n", pp.error);
        minipp_free_result(&pp);
        exit(1);
    }

    free(buffer);          // done with raw source
    buffer = pp.text;       // now buffer holds preprocessed text
    pp.text = NULL;         // ownership moved, don't let free_result touch it
    minipp_free_result(&pp);
	/* end of ai */


	Token_t *tokens = tokenize(buffer);
	instruction_t *instructions = parse(tokens);
	uint16_t *mem = code_gen(instructions);


	for(int i=0; i<65536; i++)
	{
		if(mem[i])
		{
			printf("memory[%d] = 0x%x;\n",i,mem[i]);
			outbuf[i] = mem[i];
		}
	}
	for (size_t i=0; instructions[i].instruction_type != INS_EOP; i++)
	{	
		print_instruction(instructions + i);
	}

	if (*args.outfile != 0) {
		fwrite(outbuf, 65536, 2, outfile);
	}
}

