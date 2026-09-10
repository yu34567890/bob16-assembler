#include "lex.h"
#include "token.h"
#include "utils.h"

int main()
{
    Token_t *tokens = tokenize("add 0b11111111 r1,r2,r3\na\n\n\n");
    for (int i=0; tokens[i].type; i++)
    {
        
        
        printf("%s ", token_to_string(tokens[i]));
        if (tokens[i].type == TOKEN_NUMBER)
            printf("%d",tokens[i].number);
        printf("\n");
    }
}