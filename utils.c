#include "utils.h"
#include "token.h"


char *token_to_string(Token_t token) {
    switch (token.type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_COLON: return "SEMICOLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_OPENING_BRACKET: return "TOKEN_OPENING_PAREN";
        case TOKEN_CLOSING_BRACKET: return "TOKEN_CLOSING_PAREN";
        case TOKEN_END_OF_LINE: return "TOKEN_END_OF_LINE";
        default: return "UNKNOWN";
    }
}


