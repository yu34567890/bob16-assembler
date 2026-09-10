#ifndef TOKEN_H
#define TOKEN_H
#include <stdint.h>
#include <stddef.h>

typedef enum 
{
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_OPENING_BRACKET,
    TOKEN_CLOSING_BRACKET,
    TOKEN_END_OF_LINE,
    TOKEN_UNKNOW
} Tokentype;

typedef struct Token_t
{
    Tokentype type;
    union
    {
        char* value;   
        int16_t number;
    };
    size_t index; // Position in the input string can be used to get row and column 
} Token_t;



#endif // TOKEN_H