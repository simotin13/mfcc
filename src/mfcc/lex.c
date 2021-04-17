#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "mfcc.h"
#include "lex.h"

#define KEYWORDS_MAX    (32)

// ============================================================================
// struct define
// ============================================================================
typedef struct {
    TokenType type;
    const char *str;
} KeywordsInfo;

// ============================================================================
// prototype functions
// ============================================================================
static Token *get_token(char *pos, unsigned int *len, char **nextpos, int* errcode);
static char* skip_space(char *pos, unsigned int *len);
static Token* check_hex_number(char* pos, unsigned int* len, char** nextpos, int* errcode);
static Token* check_number(char* pos, unsigned int* len, char** nextpos, int* errcode);
static Token *check_keyword_or_symbol(char *pos, unsigned int *len, char **nextpos, int* errcode);
static Token *check_keyword(char* s);
static Token *check_operator(char* code, unsigned int* len, char** nextpos, int* errcode);
static Token *new_token(TokenType type, char *val);

KeywordsInfo s_keywords[KEYWORDS_MAX] =
{
    {   T_VOID       ,       "void"     },
    {   T_CHAR       ,       "char"     },
    {   T_SHORT      ,       "short"    },
    {   T_INT        ,       "int"      },
    {   T_LONG       ,       "long"     },
    {   T_FLOAT      ,       "float"    },
    {   T_DOUBLE     ,       "double"   },
    {   T_AUTO       ,       "auto"     },
    {   T_STATIC     ,       "static"   },
    {   T_CONST      ,       "const"    },
    {   T_SIGNED     ,       "signed"   },
    {   T_UNSIGNED   ,       "unsigned" },
    {   T_EXTERN     ,       "extern"   },
    {   T_VOLATILE   ,       "volatile" },
    {   T_REGISTER   ,       "register" },
    {   T_RETURN     ,       "return"   },
    {   T_GOTO       ,       "goto"     },
    {   T_IF         ,       "if"       },
    {   T_ELSE       ,       "else"     },
    {   T_SWITCH     ,       "switch"   },
    {   T_CASE       ,       "case"     },
    {   T_DEFAULT    ,       "default"  },
    {   T_BREAK      ,       "break"    },
    {   T_FOR        ,       "for"      },
    {   T_WHILE      ,       "while"    },
    {   T_DO         ,       "do"       },
    {   T_CONTINUE   ,       "continue" },
    {   T_TYPEDEF    ,       "typedef"  },
    {   T_STRUCT     ,       "struct"   },
    {   T_ENUM       ,       "enum"     },
    {   T_UNION      ,       "union"    },
    {   T_SIZEOF     ,       "sizeof"   }
};

int tokenize(char *code, unsigned int len, Vector *tokens)
{
    int result = 0;
    unsigned int leftLen = len;
    char *pos = code;
    char *nextpos;
    Token *token = NULL;

    while(0 < leftLen) {
        pos = skip_space(pos, &leftLen);
        if (leftLen < 1) {
            break;
        }

        token = get_token(pos, &leftLen, &nextpos, &result);
        if (result == 0) {
            vec_push(tokens, token);
            pos = nextpos;
        }
    }

    return result;
}

static char* skip_space(char *pos, unsigned int *len)
{
    unsigned int leftLen = *len;
    char *nextpos = NULL;
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    DPRINT(stdout, "c:[%c], leftLen:[%d]\n", *pos, leftLen);

    while (0 < leftLen) {
        if ((*pos == ' ') || (*pos == '\t') || (*pos == '\r') || (*pos == '\n')) {
            pos++;
            leftLen--;
        }
        else {
            break;
        }
    }

    nextpos = pos;
    *len = leftLen;

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return nextpos;
}

static Token *get_token(char *pos, unsigned int *len, char **nextpos, int *errcode)
{
    Token *token = NULL;
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    token = check_number(pos, len, nextpos, errcode);
    if (token != NULL) {
        return token;
    }

    token = check_hex_number(pos, len, nextpos, errcode);
    if (token != NULL) {
        return token;
    }

    token = check_keyword_or_symbol(pos, len, nextpos, errcode);
    if (token != NULL) {
        return token;
    }

    token = check_operator(pos, len, nextpos, errcode);
    if (token != NULL) {
        return token;
    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return token;
}

static Token* check_hex_number(char* pos, unsigned int* len, char** nextpos, int* errcode)
{
    Token* token = NULL;
    unsigned int leftLen = *len;
    char sTmp[STRING_MAX];
    unsigned int sLen = 0;

    if (leftLen < 3) {
        *errcode = -1;
        return NULL;
    }
    if (pos[0] != '0') {
        *errcode = -1;
        return NULL;
    }
    if (pos[1] != 'x') {
        *errcode = -1;
        return NULL;
    }

    sTmp[0] = '0';
    sTmp[1] = 'x';
    leftLen -= 2;
    pos += 2;
    sLen += 2;

    while (leftLen) {
        if (('0' <= *pos) || (*pos <= '9')) {
            sTmp[sLen] = *pos;
            pos++;
            sLen++;
            leftLen--;
        }

        if (('A' <= *pos) || (*pos <= 'F')) {
            sTmp[sLen] = *pos;
            pos++;
            sLen++;
            leftLen--;
        }
        *errcode = -2;
        return NULL;
    }

    *errcode = 0;
    *nextpos = pos;
    *len = leftLen;
    token = new_token(T_HEX_NUMBER_LITERAL, sTmp);
    return token;
}

static Token* check_number(char* pos, unsigned int* len, char** nextpos, int* errcode) {
    char sTmp[STRING_MAX];
    Token* token = NULL;
    unsigned int sLen = 0;
    memset(sTmp, 0, STRING_MAX);
    unsigned int leftLen = *len;
    bool foundDecial = false;
    TokenType ty = T_DEC_NUMBER_LITERAL;
    if (*pos == '-') {
        // negative value
        pos++;
        leftLen--;
    }

    if (*pos == '0') {
        // must be followed by dot(.)
        sTmp[sLen++] = *pos;
        pos++;
        leftLen--;

        if (*pos != '.') {
            *errcode = -1;
            return NULL;
        }

        ty = T_FLOAT_NUMBER_LITERAL;
        foundDecial = true;
        sTmp[sLen++] = *pos;
        pos++;
        leftLen--;
    }
    if ((*pos < '1') || ('9' < *pos)) {
        // not start with 1`9
        *errcode = -1;
        return NULL;
    }

    sTmp[sLen++] = *pos;
    pos++;
    leftLen--;
    while (0 < leftLen) {
        if (isdigit(*pos)) {
            sTmp[sLen++] = *pos;
            pos++;
            leftLen--;
            continue;
        }
        if (*pos == '.') {
            if (foundDecial) {
                // decimal found more than once.
                *errcode = -2;
                return NULL;
            }
            ty = T_FLOAT_NUMBER_LITERAL;
            foundDecial = true;
            sTmp[sLen++] = *pos;
            pos++;
            leftLen--;
            if (leftLen < 1) {
                // not end with digit
                *errcode = -3;
                return NULL;
            }
            continue;
        }

        if (isalpha(*pos)) {
            return NULL;
            *errcode = -2;
        }
        break;
    }

    *errcode = 0;
    *nextpos = pos;
    *len = leftLen;

    if (sLen) {
        token = new_token(ty, sTmp);
    }

    return token;
    return NULL;

}

static Token *check_keyword_or_symbol(char *pos, unsigned int *len, char **nextpos, int *errcode)
{
    char sTmp[STRING_MAX];
    Token *token = NULL;
    unsigned int sLen = 0;
    memset(sTmp, 0, STRING_MAX);
    unsigned int leftLen = *len;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    // symbol must started with _ or alphabet
    if ((isalpha(*pos) == 0) && (*pos != '_')) {
        // Not keyword or symbol
        return NULL;
    }

    while(0 < leftLen) {
        if (isalpha(*pos) || *pos == '_' || isdigit(*pos)) {
            sTmp[sLen] = *pos;
            pos++;
            sLen++;
            leftLen--;
        } else {
            break;
        }
    }

    *errcode = 0;
    *nextpos = pos;
    *len = leftLen;

    DPRINT(stdout, "sTmp:[%s]\n", sTmp);

    token = check_keyword(sTmp);
    if (token != NULL) {
        return token;
    }

    DPRINT(stdout, "%s:%d out..\n", __FUNCTION__, __LINE__);
    return new_token(T_IDENTIFIER, sTmp);
}

static Token *check_keyword(char *s)
{
    int i;
    Token *token = NULL;
    for (i = 0; i < KEYWORDS_MAX; i++) {
        if (strcmp(s_keywords[i].str, s) == 0) {
            token = new_token(s_keywords[i].type, s);
            break;
        }
    }
    return token;
}

static Token *check_operator(char *pos, unsigned int *len, char **nextpos, int *errcode)
{
    char c = *pos;
    unsigned int leftLen = *len;
    Token *token = NULL;
    *errcode = 0;
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    switch (c)
    {
    case '+':
        token = new_token(T_PLUS, "+");
        pos++;
        leftLen--;
        break;
    case '-':
        token = new_token(T_MINUS, "-");
        pos++;
        leftLen--;
        break;
    case '*':
        token = new_token(T_ASTER, "*");
        pos++;
        leftLen--;
        break;
    case '/':
        token = new_token(T_SLASH, "/");
        pos++;
        leftLen--;
        break;
    case ',':
        token = new_token(T_COMMA, ",");
        pos++;
        leftLen--;
        break;
    case ';':
        token = new_token(T_SEMICOLON, ";");
        pos++;
        leftLen--;
        break;
    case '=':
        token = new_token(T_EQUAL, "=");
        pos++;
        leftLen--;
        break;
    case '(':
        token = new_token(T_OPEN_PAREN, "(");
        pos++;
        leftLen--;
        break;
    case ')':
        token = new_token(T_CLOSE_PAREN, ")");
        pos++;
        leftLen--;
        break;
    case '{':
        token = new_token(T_OPEN_BRACE, "{");
        pos++;
        leftLen--;
        break;
    case '}':
        token = new_token(T_CLOSE_BRACE, "}");
        pos++;
        leftLen--;
        break;
    default:
        // TODO unexpected
        *errcode = -1;
        break;
    }

    *nextpos = pos;
    *len = leftLen;

    DPRINT(stdout, "type:[%d] val:[%s]\n", token->type, token->val);
    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return token;
}

static Token *new_token(TokenType type, char *val)
{
    Token* token = malloc(sizeof(Token));
    token->type = type;
    strncpy(token->val, val, STRING_MAX);
    token->val[STRING_MAX -1] = 0;
    return token;
}
