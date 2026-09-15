#include "lex.h"
#include "token.h"
#include <ctype.h>

int
atoi_n (const char *nptr, int8_t base)
{
  return (int) strtol (nptr, (char **) NULL, base);
}

Token_t* tokenize(char* input) { // todo add comments eg //
	Token_t* result = malloc((strlen(input)+222) * sizeof(Token_t)); // number of tokens must be smaller than the ammount of charachters 
	size_t token_pos = 0;
	size_t index = 0;
	for(; input[index]; index++)
	{
	uint8_t is_negative = 0; 

		if((input[index]) == '\n' || input[index] == '\r')
		{
			result[token_pos].type = TOKEN_END_OF_LINE;
			result[token_pos].index = index;
			result[token_pos++].value = strdup(")");
		}

		else if (isspace(input[index]));
	else if (input[index] == ','); // for the nasm preprocessor
	
	else if (input[index] == ';')
	{
		for(; input[index] != '\n' && input[index]; index++);
		index--;
	}

	else if (input[index] == '%') // for the nasm preprocessor please dont use this shit in the asm code 
	{
		for(; input[index] != '\n' && input[index]; index++);
		index--;
	}
	
	else if (input[index] == '-')
	{
		is_negative = 1;
		if (isdigit(input[index+1]))
		{
			index++;
			if(input[index] == '0')
			{
				goto HANDLE_ME;
			}
			else 
			{
				goto HANDLE_ZERO;
			}
		}
		else 
		{
			printf("il negate you if you keep this up");
			exit(0xff);
		}
	}

		else if(input[index] == '0')
		{
		HANDLE_ME:
		if (input[index+1] == 'x')
		{
			index+=2;
			size_t index2=index;
			for(; isxdigit((unsigned char)input[index2]); index2++);
			size_t len = index2 - index;
			char* matched = malloc(len+1);
			memcpy(matched, input + index, len);
			matched[len] = 0;
			result[token_pos].type = TOKEN_NUMBER;
			result[token_pos].index = index;
			result[token_pos++].number = (is_negative) ? (is_negative=0,-atoi_n(matched, 16)) : atoi_n(matched, 16);
			index = index2-1; 
		}
				
		else if (input[index+1] == 'b')
		{
			index+=2;
			size_t index2=index;
			for(; isdigit((unsigned char)input[index2]); index2++);
			size_t len = index2 - index;
			char* matched = malloc(len+1);
			memcpy(matched, input + index, len);
			matched[len] = 0;
			result[token_pos].type = TOKEN_NUMBER;
			result[token_pos].index = index;
			result[token_pos++].number = (is_negative) ? (is_negative=0,-atoi_n(matched, 2)) : atoi_n(matched, 2);
			index = index2-1;
		}
		else 
		{
			goto HANDLE_ZERO;
		}
			
		}

		else if(isdigit((unsigned char)input[index]))
		{
			HANDLE_ZERO:
			size_t index2=index;
			for(; isdigit((unsigned char)input[index2]); index2++);
			size_t len = index2 - index;
			char* matched = malloc(len+1);
			memcpy(matched, input + index, len);
			matched[len] = 0;
			
			result[token_pos].type = TOKEN_NUMBER;
			result[token_pos].index = index;
			result[token_pos++].number = (is_negative) ? (is_negative=0,-atoi_n(matched, 10)) : atoi_n(matched, 10);

			
			index = index2-1;
			continue;
		}

		else if(isalpha((unsigned char)input[index]))
		{
			size_t index2=index;
			for(; isalnum((unsigned char)input[index2]) || input[index2] == '.' || input[index2] == '_'; index2++);
			size_t len = index2 - index;
			char* matched = malloc(len+1);
			memcpy(matched, input + index, len);
			matched[len] = 0;

			result[token_pos].type = TOKEN_IDENTIFIER;
			result[token_pos].index = index;
			result[token_pos++].value = matched;
			
			
			index = index2-1;
			continue;
		}

		else if(input[index] == ':')
		{
				result[token_pos].type = TOKEN_COLON;
				result[token_pos].index = index;
				result[token_pos++].value = strdup(";");
		}


		else if(input[index] == ',')
		{
			result[token_pos].type = TOKEN_COMMA;
			result[token_pos].index = index;
			result[token_pos++].value = strdup(",");
		}
		else if(input[index] == '[')
		{
			result[token_pos].type = TOKEN_OPENING_BRACKET;
			result[token_pos].index = index;
			result[token_pos++].value = strdup("(");
		}
		else if(input[index] == ']')
		{
			result[token_pos].type = TOKEN_CLOSING_BRACKET;
			result[token_pos].index = index;
			result[token_pos++].value = strdup(")");
		}

		else 
		{
			result[token_pos].type = TOKEN_UNKNOW;
			result[token_pos].index = index;
			result[token_pos].value = malloc(2);
			result[token_pos].value[1] = 0;
			result[token_pos++].value[0] = input[index];
		}
	}
  	if (token_pos > 0 && result[token_pos-1].type != TOKEN_END_OF_LINE) {
  		result[token_pos].type = TOKEN_END_OF_LINE;
  		result[token_pos].index = index;
  		result[token_pos++].value = strdup(")");
  	}
  	
  	
	result[token_pos].type = TOKEN_EOF;
	result[token_pos].value = malloc(1); 
	result[token_pos].value[0] = 0;
	result[token_pos].index = index;
	
	Token_t* tmp = realloc(result,(token_pos+1)*sizeof(struct Token_t));
	if(tmp!=NULL)
	{
		
		return tmp;
	}
	else
	{
		perror("unable to realloc memory for tokens using old");
	}

	return result; 
}
