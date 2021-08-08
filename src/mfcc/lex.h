#ifndef _LEX_H_
#define _LEX_H_

#include "mfcc.h"
#include "vector.h"

// ============================================================================
// enum define
// ============================================================================
typedef enum
{
    T_DEC_NUMBER_LITERAL = 0,
    T_HEX_NUMBER_LITERAL,
    T_FLOAT_NUMBER_LITERAL,
    T_STRING_LITERAL,
    T_CHAR_LITERAL,
    T_PLUS,
    T_MINUS,
    T_SLASH,
    T_ASTER,
    T_COMMA,
    T_SEMICOLON,
    T_EQUAL,
    T_IDENTIFIER,
    T_OPEN_PAREN,       // (
    T_CLOSE_PAREN,      // )
    T_OPEN_BRACE,       // {
    T_CLOSE_BRACE,      // }
    // keywords
    T_VOID,
    T_CHAR,
    T_SHORT,
    T_INT,
    T_LONG,
    T_FLOAT,
    T_DOUBLE,
    T_AUTO,
    T_STATIC,
    T_CONST,
    T_SIGNED,
    T_UNSIGNED,
    T_EXTERN,
    T_VOLATILE,
    T_REGISTER,
    T_RETURN,
    T_GOTO,
    T_IF,
    T_ELSE,
    T_SWITCH,
    T_CASE,
    T_DEFAULT,
    T_BREAK,
    T_FOR,
    T_WHILE,
    T_DO,
    T_CONTINUE,
    T_TYPEDEF,
    T_STRUCT,
    T_ENUM,
    T_UNION,
    T_SIZEOF,
} TokenType;

// ============================================================================
// struct define
// ============================================================================
typedef struct _Token {
    int type;
    char val[STRING_MAX];
} Token;

// ============================================================================
// prototype functions
// ============================================================================
extern int tokenize(char *code, unsigned int len, Vector *tokens);
extern char* type2str(TokenType);
#endif  // _LEX_H_