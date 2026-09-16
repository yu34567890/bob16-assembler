#include "instruction.h"
#include "machine_code.h"
#define sext_4(x)  (((x) >= 0x8)   ? (int16_t)((x) | 0xFFF0) : (x))
#define sext_6(x)  (((x) >= 0x20)  ? (int16_t)((x) | 0xFFC0) : (x))
#define sext_7(x)  (((x) >= 0x40)  ? (int16_t)((x) | 0xFF80) : (x))
#define sext_9(x)  (((x) >= 0x100) ? (int16_t)((x) | 0xFE00) : (x))
#define sext_11(x) (((x) >= 0x400) ? (int16_t)((x) | 0xF800) : (x))


typedef struct fake_hashmap
{
	char *label;	
	uint16_t location;
} HashmapLarp;

enum {
	NOP,
	ADD,
	AND,
	NOT,
	LD,
	LDI,
	LDR,
	ST,
	STI,
	STR,
	BR,
	JMP,
	JSR,
	LEA,
	RET,
	TRAP
} ;


uint16_t *code_gen(instruction_t *instructions)
{
	// first pass for the labels
	uint16_t *memory = calloc(1,65536*sizeof(uint16_t));
	uint16_t pc = 0;
	uint16_t label_track=0;
	HashmapLarp label_map[65536]; // if you have more labels than 65536 goto an hospital you damn insane man 

	for (size_t i=0; instructions[i].instruction_type != INS_EOP; i++) 
	{
		if (instructions[i].instruction_type == LABEL)
		{
			label_map[label_track].location = pc;
			label_map[label_track++].label = instructions[i].label;
		}

		else if (instructions[i].instruction_type == ORG)
		{
			pc = instructions[i].imm;
		}
		
		else if (instructions[i].instruction_type>=INS_SUB && instructions[i].instruction_type < LABEL) // after INS_SUB and including sub they use 2 opcodes
		{
			printf("shit ass instruction found reported this is a bug");
			pc+=2;
		}
		else
		{
			pc+=1;
		}
	}

	// second pass the compilation god help me 
	pc = 0;
	for (size_t i=0; instructions[i].instruction_type != INS_EOP; i++) 
	{
		if (instructions[i].instruction_type>=INS_SUB && instructions[i].instruction_type != LABEL)
		{
			pc+=2;
		}
		else
		{
			switch(instructions[i].instruction_type)
			{ 
			// (current_instruction >> 1) src1
			// (current_instruction >> 4) src2
			// (current_instruction >> 9) dst
				case ORG:
					pc = instructions[i].imm;
					continue;	
					break;
				
				case WORD:
					if (instructions[i].label)
					{
						if (instructions[i].label != NULL)
						{
						for (int j = 0; j < label_track; j++)
						{
							if (strcmp(label_map[j].label, instructions[i].label) == 0)
							{
							memory[pc] = label_map[j].location;
							
							break;
							}
						}
						}	
						
					}
					else
					{
						memory[pc]=instructions[i].imm;
					}
					break;

				case INS_TRAP:
					memory[pc]= (TRAP<<12) | ((instructions[i].imm&0xf)<<8);
					break;
			
				case INS_NOP:
					memory[pc]=0;
					break;

				case INS_ADD_RR: 
					memory[pc] = (ADD<<12) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4) + 
					(instructions[i].src2<<1);
					break;

				case INS_ADD_RI4:
					memory[pc] = (ADD<<12) + (1<<7) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4) + 
					(instructions[i].imm&0xF);
					break;

				case INS_ADD_RR2:
					memory[pc] = (ADD<<12) + (2<<7) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4);
					break;

				case INS_ADD_RI7:
					memory[pc] = (ADD<<12) | (3<<7) | (instructions[i].dst<<9) | 
					(instructions[i].imm & 0x7f);					
					break;


				case INS_AND_RR: 
					memory[pc] = (AND<<12) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4) + 
					(instructions[i].src2<<1);
					break;

				case INS_AND_RI4:
					memory[pc] = (AND<<12) + (1<<7) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4) + 
					(instructions[i].imm&0xF);
					break;

				case INS_AND_RR2:
					memory[pc] = (AND<<12) + (2<<7) + (instructions[i].dst<<9) + 
					(instructions[i].src1<<4);
					break;

				case INS_AND_RI7:
					memory[pc] = (AND<<12) + (3<<7) + (instructions[i].dst<<9) + 
					(instructions[i].imm & 0x7f);					
					break;


				case INS_NOT_RR:
					memory[pc] = (NOT<<12) +
					(instructions[i].dst << 9) +
					(instructions[i].src1<< 5);
					break;

				case INS_NOT_R:
					memory[pc] = (NOT<<12)+
					(instructions[i].dst<<9) + (1<<8);
					break;
												
				case INS_LD:
					memory[pc] = (LD<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].imm&0x1ff);	
					break;

				case INS_LDI:
					memory[pc] = (LDI<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].imm&0x1ff);	
					break;

				case INS_LDR:
					memory[pc] = (LDR<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].src1<<6)+
					(instructions[i].imm&0x3f);
					break;



				case INS_ST:
					memory[pc] = (ST<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].imm&0x1ff);	
					break;

				case INS_STI:
					memory[pc] = (STI<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].imm&0x1ff);	
					break;

				case INS_STR:
					memory[pc] = (STR<<12)|
					(instructions[i].dst<<9)|
					(instructions[i].src1<<6)|
					(instructions[i].imm&0x3f);
					break;


				case INS_BR: 
				{
					int jmp_adress = 0;
					if (instructions[i].label != NULL)
					{
						for (int j = 0; j < label_track; j++)
						{
							if (strcmp(label_map[j].label, instructions[i].label) == 0)
							{
								jmp_adress = (int)label_map[j].location - (int)pc - 1;
				
								if (jmp_adress > 255 || jmp_adress < -256)
								{
									printf("label is too far to the br instruction\n");
									exit(0x38);
								}
								break; // also worth adding — no need to keep scanning once matched
							}
						}
					}
					else
					{
						jmp_adress = instructions[i].imm;
					}
					jmp_adress &= 0b111111111;
					memory[pc] = (BR << 12) +
								 ((instructions[i].dst & 0b111) << 9) +
								 jmp_adress;
					break;
				}

				case INS_JMP_REG:
					memory[pc] = (JMP<<12)+
					(instructions[i].dst<<9);
					break;

				case INS_JSR_REL:
					int jmp_adress2;
					if (instructions[i].label != NULL)
					{
						for(int label_i=0; label_i<label_track; label_i++)
						{
							
							if(strcmp(label_map[label_i].label, instructions[i].label)==0)
							{
								jmp_adress2 =
										(int)label_map[label_i].location - (int)pc - 1;
								if (jmp_adress2 > 1023 || jmp_adress2 < -1024)
								{
									printf("label is too far to the jsr instruction");
									exit(02);
								}
								goto FOUND;								
							}
						}
						
						printf("label %s not found ", instructions[i].label);
						exit(26672);
						FOUND:		
						
						
					}
					else
					{
						jmp_adress2 = instructions[i].imm;
					}
					
					jmp_adress2 =  jmp_adress2 & 0x7FF;
					memory[pc] = ((JSR & 0xF) << 12) | (jmp_adress2) ;
					break;

				case INS_JSR_REG:
					
					memory[pc] = (JSR<<12) |
					(instructions[i].dst<<8) | 0x800;
					break;

				case INS_LEA:
					memory[pc] = (LEA<<12)+
					(instructions[i].dst<<9)+
					(instructions[i].imm&0b111111111);
					break;

				case INS_RET:
					memory[pc] = (RET<<12);
					break;

				
				default:
					continue;
					break;
			}
			pc++;

		}
		
	}
	return memory;
	
}
