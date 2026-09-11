#include "instruction.h"


/*
int parseReg(const char* s) {
	if (!s) {
		//string doesn't exist
		return -1;
	}

	if (strlen(s) != 2 || s[0] != 'R' && s[0] != 'r') {
		//incorrect format
		return -2;
	}

	if (s[1] < '0' || s[1] > '7') {
		//out of bounds
		return -3;
	}

	return s[1] - '0';
}
*/



int parse_reg(Token_t token)
{
	if (token.type!=TOKEN_IDENTIFIER)
	{
		return -1;
	}

	if (!token.value)
	{
		return -2;
	}

	if (strlen(token.value) < 2)
	{
		return -3;
	}

	if (token.value[0] != 'R' && token.value[0] != 'r')
	{
		return -5;
	}

	if (token.value[1] < '0' || token.value[1] > '7') 
	{
		return -6;
	}

	return token.value[1] - '0';
}



instruction_t *parse(Token_t *tokens)
{

	fflush(stdout);
	size_t pc=0;
	size_t instruction_pos=0;
	size_t token_len=0;
	for(; tokens[token_len].type!=TOKEN_EOF; token_len++);
	
	instruction_t *instructions = calloc(1,sizeof(instruction_t) * (token_len + 5));
	
	for (; pc < token_len; pc++)
	{
		
		if (tokens[pc].type == TOKEN_END_OF_LINE) continue; // empty line
		Token_t dst={0};
		Token_t src1={0};
		Token_t src2={0};
		Token_t instruction={0}; // could be an label too 
		
		uint8_t is_label=0;
		uint8_t instruction_length=0;
		size_t i=0;
		
		for (;!(tokens[i+pc].type == TOKEN_END_OF_LINE || tokens[i+pc].type == TOKEN_EOF); i++) // retrive instruction	
		{
			switch (i)
			{
				case 0: // expect id
					if (tokens[i+pc].type !=  TOKEN_IDENTIFIER)
					{
						printf("unexpected token");
						exit(0xdeadbeef);
					}
					instruction = tokens[i+pc];
					break;
					
				case 1: // expect id or colon aka label
					if (tokens[i+pc].type == TOKEN_COLON) // ths sht is a label
					{
						is_label=1;
						break;
					}
					
					if (tokens[i+pc].type !=  TOKEN_IDENTIFIER && tokens[i+pc].type!=TOKEN_NUMBER)
					{
						printf("unexpected token");
						exit(0xdeadbeef);
					}
					dst = tokens[i+pc];
					break;
					
				case 2:
					// if label give error
					if (is_label)
					{
						printf("unexpected token after label");
						exit(0xdeadbeef);
					}
					// maybe dont expect , since theres no meaningfull purpose for it
					if (tokens[i+pc].type != TOKEN_IDENTIFIER && tokens[i+pc].type != TOKEN_NUMBER)
					{
						printf("unexpected token");
						exit(0xdeadbeef);
					}

					src1 = tokens[i+pc];
					break;

				case 3:
					if (tokens[i+pc].type != TOKEN_IDENTIFIER && tokens[i+pc].type != TOKEN_NUMBER)
					{
						printf("unexpected token");
						exit(0xdeadbeef);
					}

					src2 = tokens[i+pc];
					break;
			}
		}
		
		instruction_length = i-1;
		pc += i;
		if (is_label)
		{
			instructions[instruction_pos].instruction_type = LABEL;
			printf(" label found %s\n", tokens[pc-i].value);
			instructions[instruction_pos++].label = tokens[pc-i].value;
			continue;
		}

		if (!strcmp(instruction.value, "br"))
		{

			if (instruction_length != 2)
			{
				printf(" get out 💔🥀");
				exit(0x1F9400A);
			}
			if (dst.type != TOKEN_NUMBER)
			{
				printf("bro i cant deal with this shit anymore😭😭😭");
			}
			
			if (parse_reg(src1) == -5) // not a register so its a label
			{
				instructions[instruction_pos].instruction_type = INS_BR;
				instructions[instruction_pos].src1 = dst.number; // let memory leak we have gigabytes of memory anyway
				instructions[instruction_pos++].label = src1.value;
			}
			
			else if (src1.type == TOKEN_NUMBER)
			{
				instructions[instruction_pos].instruction_type = INS_BR;
				instructions[instruction_pos].src1 = dst.number; // let memory leak we have gigabytes of memory anyway
				instructions[instruction_pos++].imm = src1.number;
			}

			else
			{
				printf("what are you even doing atp ");
				exit(02466);
			}
			continue;
		}

		else if (!strcmp(instruction.value, "jsr")) // :(
		{
			if (dst.type == TOKEN_NUMBER)
			{
				instructions[instruction_pos].instruction_type = INS_JSR_REL;
				instructions[instruction_pos++].imm = dst.number;
			}
			
			else if(dst.type == TOKEN_IDENTIFIER)
			{
				if (parse_reg(dst) == -5)
				{
					instructions[instruction_pos].instruction_type = INS_JSR_REL;
					instructions[instruction_pos++].label = dst.value;
					continue;
				}
				instructions[instruction_pos].instruction_type = INS_JSR_REG;
				instructions[instruction_pos++].dst = parse_reg(dst);
				continue;
			}
			
			else
			{
				printf("no more insulting 💔");
				exit(35);
			}
		}
		
		if(parse_reg(dst)<0)
		{
			printf(" %s ", dst.value);
			printf("couldnt parse dst register");
			exit(0xdeadbeef);
		}

		if(src1.type==TOKEN_IDENTIFIER && parse_reg(src1)<0)
		{
			printf("couldnt parse src1 register");
			exit(0xdeadbeef);
		}
		
		if(src2.type==TOKEN_IDENTIFIER && parse_reg(src2)<0)
		{
			printf("couldnt parse src2 register");
			exit(0xdeadbeef);
		}

		// time to do long ass string comparissions :sob:

		if (!strcmp(instruction.value, "nop"))
		{
			if (instruction_length!=0)
			{
				printf("nop instruction does not expect any args");
				exit(0xdeadbeef);
			}
			instructions[instruction_pos++].instruction_type = INS_NOP;
		}
		
		else if (!strcmp(instruction.value, "add"))
		{
			if (instruction_length==2) // either add r1 r2 or add r1 imm
			{
				if (src1.type == TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_ADD_RI7;
					instructions[instruction_pos].imm=src1.number;
					instructions[instruction_pos++].dst=parse_reg(dst);
				}
				
				else if (src1.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_ADD_RR2;
					instructions[instruction_pos].imm=src1.number;
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}				
			}

			else if (instruction_length==3) // add Rd, Rs, Rs or ADD Rd, Rs, imm
			{
				if (src1.type==TOKEN_IDENTIFIER && src2.type==TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_ADD_RI4;
					instructions[instruction_pos].imm=src2.number;
					instructions[instruction_pos].src1=parse_reg(src1);
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}
				
				else if (src1.type==TOKEN_IDENTIFIER && src2.type==TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_ADD_RR;
					instructions[instruction_pos].src2=parse_reg(src2);
					instructions[instruction_pos].src1=parse_reg(src1);
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}

				else
				{
					printf("instruction invalid operands");
					exit(0xdeadbeef);
				}
				
			}
			
			else
			{
				printf("instruction invalid operands");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "and"))
		{
			if (instruction_length==2) // either add r1 r2 or add r1 imm
			{
				if (src1.type == TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_AND_RI7;
					instructions[instruction_pos].imm=src1.number;
					instructions[instruction_pos++].dst=parse_reg(dst);
				}
				
				else if (src1.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_AND_RR2;
					instructions[instruction_pos].imm=src1.number;
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}				
			}

			else if (instruction_length==3) // add Rd, Rs, Rs or ADD Rd, Rs, imm
			{
				if (src1.type==TOKEN_IDENTIFIER && src2.type==TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_AND_RI4;
					instructions[instruction_pos].imm=src2.number;
					instructions[instruction_pos].src1=parse_reg(src1);
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}
				
				else if (src1.type==TOKEN_IDENTIFIER && src2.type==TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_AND_RR;
					instructions[instruction_pos].src2=parse_reg(src2);
					instructions[instruction_pos].src1=parse_reg(src1);
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}

				else
				{
					printf("instruction invalid operands");
					exit(0xdeadbeef);
				}
				
			}
			else
			{
				printf("instruction invalid operands");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "not"))
		{
			if (instruction_length==2)
			{
				if (src1.type == TOKEN_NUMBER) // AHHHHHH
				{
					instructions[instruction_pos].instruction_type = INS_NOT_RI4;
					instructions[instruction_pos].imm=parse_reg(src1);
					instructions[instruction_pos++].dst=parse_reg(dst);	
				}

				else if (src1.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_NOT_RR;
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos++].dst = parse_reg(dst);									
				}
				
				else	
				{
					printf("f you");
					exit(0xdeadbeef);
				}
			}
			else
			{
				printf("wrong not dumbass");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "ld"))
		{
			if(instruction_length==2)
			{
				if (src1.type != TOKEN_NUMBER)
				{
					printf("hairy balls");
					exit(0xball);
				}
				instructions[instruction_pos].instruction_type = INS_LD;
				instructions[instruction_pos].imm = src1.number;
				instructions[instruction_pos++].dst = parse_reg(dst);											
				
			}
			else
			{
				printf("nah222");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "ldi"))
		{
			if(instruction_length==2)
			{
				if (src1.type != TOKEN_NUMBER)
				{
					printf("hairy balls 2.0");
					exit(0x2ball);
				}
				instructions[instruction_pos].instruction_type = INS_LDI;
				instructions[instruction_pos].imm = src1.number;
				instructions[instruction_pos++].dst = parse_reg(dst);											
				
			}
			else
			{
				printf("nah1222");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "ldr"))
		{
			if(instruction_length==3)
			{
				if (src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_LDR;
					instructions[instruction_pos].imm = src2.number;
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos++].dst = parse_reg(dst);
				}
				else
				{
					printf("nahh1234");
					exit(0xdeadbell);
				}
			}
			else
			{
				printf("bro>:(");
				exit(0xb32666);
			}
		}
		
		else if (!strcmp(instruction.value, "st"))
		{
			if(instruction_length==2)
			{
				if (src1.type != TOKEN_NUMBER)
				{
					printf("hairy balls");
					exit(0xball);
				}
				instructions[instruction_pos].instruction_type = INS_ST;
				instructions[instruction_pos].imm = src1.number;
				instructions[instruction_pos++].dst = parse_reg(dst);											
				
			}
			else
			{
				printf("nah222");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "sti"))
		{
			if(instruction_length==2)
			{
				if (src1.type != TOKEN_NUMBER)
				{
					printf("hairy balls 2.0");
					exit(0x2ball);
				}
				instructions[instruction_pos].instruction_type = INS_STI;
				instructions[instruction_pos].imm = src1.number;
				instructions[instruction_pos++].dst = parse_reg(dst);											
				
			}
			else
			{
				printf("nah1222");
				exit(0xdeadbeef);
			}
		}
		
		else if (!strcmp(instruction.value, "str"))
		{
			if(instruction_length==3)
			{
				if (src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_NUMBER)
				{
					instructions[instruction_pos].instruction_type = INS_STR;
					instructions[instruction_pos].imm = src2.number;
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos++].dst = parse_reg(dst);
				}
				else
				{
					printf("nahh1234");
					exit(0xdeadbell);
				}
			}
			else
			{
				printf("bro>:(");
				exit(0xb32666);
			}
		}
		
		else if (!strcmp(instruction.value, "jmpr"))
		{
			if (instruction_length != 1)
			{
				printf("get your bitch ass out of here");
				exit(0x2);
			}

			instructions[instruction_pos].instruction_type = INS_JMP_REG;
			instructions[instruction_pos++].dst=parse_reg(dst);
		}
		
		
		else if (!strcmp(instruction.value, "lea"))
		{
			if (instruction_length != 3)
			{
				printf("are you dumbass");
				exit(0x531);
			}

			if (src1.type != TOKEN_NUMBER)
			{
				printf("yes youre a ndumbass");
				exit(0x51);
			}
			
			instructions[instruction_pos].instruction_type = INS_LEA;
			instructions[instruction_pos].imm = src1.number;
			instructions[instruction_pos].dst = parse_reg(dst);
		}
		
		else if (!strcmp(instruction.value, "ret"))
		{
			if (instruction_length != 0)
			{
				printf("dude just rm -rf yourself");
				exit(38931);
			}
			instructions[instruction_pos++].instruction_type = INS_RET;
		}
		// will be implemented later
		
		else if (!strcmp(instruction.value, "sub"))
		{
		// god dayum

			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_SUB;
					if ((parse_reg(dst) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
			}
			else
					{
						printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
						exit(0x24);
					}
		}
		
		else if (!strcmp(instruction.value, "mul"))
		{
			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_MUL;
					if ((parse_reg(dst) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
			}
			else
					{
						printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
						exit(0x24);
					}
		}
		
		else if (!strcmp(instruction.value, "idiv"))
		{
			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_IDIV;
					if ((parse_reg(dst) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
			}
			else
					{
						printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
						exit(0x24);
					}
		}
		
		else if (!strcmp(instruction.value, "mod"))
		{
			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_MOD;
					if ((parse_reg(dst) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
			}
			else
			{
				printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
				exit(0x24);
			}
		}
		
		else if (!strcmp(instruction.value, "inc"))
		{
			if (instruction_length == 1)
			{
				instructions[instruction_pos].dst = parse_reg(dst);
				instructions[instruction_pos++].instruction_type = INS_INC;
			}
			else
			{
				printf("be a good boy and obey my rules");
				exit(0xb08);
			}
		}
		
		else if (!strcmp(instruction.value, "dec"))
		{
			if (instruction_length == 1)
			{
				instructions[instruction_pos].dst = parse_reg(dst);
				instructions[instruction_pos++].instruction_type = INS_DEC;
			}
			else
			{
				printf("be a good boy and obey my rules");
				exit(0xb08);
			}
		}
		
		else if (!strcmp(instruction.value, "min"))
		{
			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_MIN;
					if ((parse_reg(dst) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
				else
				{
					printf("fUc& you man");
					exit(0xffff);
				}
			}
			else
			{
				printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
				exit(0x24);
			}

		}
		
		else if (!strcmp(instruction.value, "max"))
		{
			if (instruction_length == 3)
			{
				if (dst.type == TOKEN_IDENTIFIER && src1.type == TOKEN_IDENTIFIER && src2.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_MAX;
					if ((parse_reg(src1) < 0|| parse_reg(src1) < 0|| parse_reg(src2) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos].src2 = parse_reg(src2);
					instructions[instruction_pos++].dst = parse_reg(dst);
					continue;													
										
				}
				else
				{
					printf("fUc& you man");
					exit(0xffff);
				}
			}
			else
			{
				printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
				exit(0x24);
			}
		}
		
		else if (!strcmp(instruction.value, "cmp"))
		{
			if (instruction_length == 2)
			{
				if (src1.type == TOKEN_IDENTIFIER && dst.type == TOKEN_IDENTIFIER)
				{
					instructions[instruction_pos].instruction_type = INS_CMP;
					if ((parse_reg(src1) < 0|| parse_reg(dst) < 0))
					{
						printf("THY END IS NOW");
						exit(0x24);
					}
					instructions[instruction_pos].src1 = parse_reg(src1);
					instructions[instruction_pos++].dst = parse_reg(dst);
					
					continue;													
										
				}
				
			}
			else
			{
				printf("THE CRIMES THY KIND HAS COMMITED AGAISNT THIS ASSEMBLER WILL NOT BE FORGIVEN");
				exit(0x24);
			}
		}
		else
		{
			printf("i dont even know what did you just tpyed");
		}
	}
	instructions[instruction_pos++].instruction_type = INS_EOP;
	return instructions;
}
