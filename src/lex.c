#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "gpcc.h"
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
static Token *get_token(char *pos, unsigned int *len, char **nextpos, int *errcode);
static char* skip_space(char *pos, unsigned int *len);
static Token *check_number(char *pos, unsigned int *len, char **nextpos, int *errcode);
static Token *check_keyword_or_symbol(char *pos, unsigned int *len, char **nextpos, int *errcode);
static Token *check_keyword(char *s);
static Token *check_operator(char *code, unsigned int *len, char **nextpos, int *errcode);
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
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    while(0 < leftLen) {
        DPRINT(stdout, "scan loop, leftLen:[%d], *pos:[%c]\n", leftLen, *pos);
        pos = skip_space(pos, &leftLen);
        if (leftLen < 1) {
            break;
        }

        token = get_token(pos, &leftLen, &nextpos, &result);
        if (result == 0) {
            vec_push_back(tokens, token);
            pos = nextpos;
        }
    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
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

static Token *check_number(char *pos, unsigned int *len, char **nextpos, int *errcode)
{
    Token *token = NULL;
    unsigned int leftLen = *len;
    char sTmp[STRING_MAX];
    unsigned int sLen = 0;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    memset(sTmp, 0, STRING_MAX);

    // TODO Hex, Oct format string
    while(0 < leftLen) {
         if (isdigit(*pos) == 0) {
             break;
         }
         sTmp[sLen] = *pos;
         pos++;
         sLen++;
         leftLen--;
    }

    *nextpos = pos;
    *len = leftLen;

    if (sLen) {
    	printf("%s!\n", sTmp);
        token = new_token(T_INTEGER, sTmp);
    	DPRINT(stdout, "sTmp:[%s]\n", sTmp);
    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return token;
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
