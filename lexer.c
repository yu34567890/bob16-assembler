#include "lex.h"
#include "token.h"
#include <ctype.h>

int
atoi_n (const char *nptr, int8_t base)
{
  return (int) strtol (nptr, (char **) NULL, base);
}

Token_t* tokenize(char* input) { // todo add comments eg //
    Token_t* result = malloc((strlen(input)+1) * sizeof(Token_t)); // number of tokens must be smaller than the ammount of charachters 
    size_t token_pos = 0;
    size_t index = 0;
    for(; input[index]; index++)
    {
        

        if((input[index]) == '\n')
        {
            result[token_pos].type = TOKEN_END_OF_LINE;
            result[token_pos].index = index;
            result[token_pos++].value = strdup(")");
        }

        else if (isspace(input[index]));
        
        else if(input[index] == '0')
        {
            HANDLE_THIS_SHIT:
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
                result[token_pos++].number = atoi_n(matched, 16);
                index = index2;
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
                result[token_pos++].number = atoi_n(matched, 2);
                index = index2;
            }
            
        }

        else if(isdigit((unsigned char)input[index]))
        {
            size_t index2=index;
            for(; isdigit((unsigned char)input[index2]); index2++);
            size_t len = index2 - index;
            char* matched = malloc(len+1);
            memcpy(matched, input + index, len);
            matched[len] = 0;
            
            result[token_pos].type = TOKEN_NUMBER;
            result[token_pos].index = index;
            result[token_pos++].number = atoi(matched);
            
            index = index2-1;
            continue;
        }
        else if(isalpha((unsigned char)input[index]))
        {
            size_t index2=index;
            for(; isalnum((unsigned char)input[index2]) || input[index2] == '.'; index2++);
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