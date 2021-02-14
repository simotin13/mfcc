#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lex.h"
#include "ast.h"
#include "type.h"
#include "variable.h"
#include "func.h"
#include "scope.h"

typedef enum {
    DECL_TYPE_VAR,
    DECL_TYPE_FUNCTION_PROTOTYPE,
    DECL_TYPE_FUNCTION_BODY,
} DeclType;

typedef struct {
    Scope scope;
    unsigned int ptr;
    char sym[STRING_MAX];
} VarInfo;

typedef struct {
    char name[STRING_MAX];
    Type retType;
    Vector* args;
    Vector* nodes;
} FuncInfo;

// ============================================================================
// prototype functions
// ============================================================================
static Program *parse_toplevel(int *ec);
static int parse_declare_specifier(DeclType *declType, void *decl);
static int parse_function_args(Vector *args);
static int parse_function_body(FuncDecl* func);
static int parse_variable(Variable* var);

static void initialzize_types(void);
static Token* cur_token();
static bool is_type_of(Token *t, Type *types, int len);
static bool is_token_type(Token* t, TokenType tokenType);
static bool is_literal_or_of(Token *t, Variable *var);
static bool is_c_types(Token* t);
static Token *consume(void);
static void error_unexpected_token(TokenType expected);

// token info
static Token* s_tokens = NULL;
static int s_token_pos = 0;
static int s_token_len = 0;

// types info
static const Type c_types[] =
{
    {   0,  "void"   },
    {   1,  "char"     },
    {   2,  "short"    },
    {   3,  "int"      },
    {   4,  "long"     },
    {   5,  "float"    },
    {   6,  "double"   },
};
#define C_TYPES_LEN     (7)
Type* s_types = NULL;
static int s_types_len;

int parse_tokens(Vector *tokens, Program *program)
{
    int ec;
    int i;
    Token* token;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    
    // initialize types info
    initialzize_types();
    
    // copy tokens
    s_tokens = malloc(sizeof(Token) * tokens->size);
    for (i = 0; i < tokens->size; i++) {
        token = (Token *)tokens->data[i];
        memcpy(&s_tokens[i], token, sizeof(Token));
    }

    s_token_pos = 0;
    s_token_len = tokens->size;
    program = parse_toplevel(&ec);

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return ec;
}

static void initialzize_types(void)
{
    int i;
    s_types = malloc(sizeof(Type) * C_TYPES_LEN);
    for (i = 0; i < C_TYPES_LEN; i++) {
        memcpy(&s_types[i], &c_types[i], sizeof(Type));
    }
    s_types_len = C_TYPES_LEN;
    return;
}

static Program *parse_toplevel(int *ec)
{
    int result = 0;
    Program *program;
    DeclType declType;
    FuncDecl* func = NULL;
    FuncBody* funcBody = NULL;
    void* decl = NULL;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    while(s_token_pos < s_token_len) {
        result = parse_declare_specifier(&declType, decl);
        if (result != 0) {
            return -1;
        }
        if (declType == DECL_TYPE_VAR) {
            // TODO
        } else if (declType == DECL_TYPE_FUNCTION_PROTOTYPE) {
            // TODO
        } else if (declType == DECL_TYPE_FUNCTION_BODY) {
            func = decl;
            result = parse_function_body(func, funcBody);
            if (result != 0) {
                return -1;
            }
        }
        #if 0
        result = declare_function(&varInfo);
        if (result == 0) {
            // var found
            DPRINT(stdout,
                "variable found scope:[%d}, DataType:[%d] pointer:[%d], symbol:[%s] in...\n",
                varInfo.scope, varInfo.dt, varInfo.ptr, varInfo.sym);
            continue;
        }
        #endif

    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);

    // TODO
    return NULL;
}

static int parse_declare_specifier(DeclType *declType, void *decl) {
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    bool bResult;
    int result;
    Token* t;
    Token* sym;
    Variable* var;
    Vector* vars;
    FuncDecl *funcDecl;
    vars = vec_new();

    t = cur_token();
    bResult = is_type_of(t, s_types, s_types_len);
    if (bResult != true) {
        return -1;
    }
    var = variable_new(t->val, t->type, 0);

    sym = consume();
    bResult = is_token_type(sym, T_IDENTIFIER);
    if (bResult != true) {
        return -1;
    }

    t = consume();
    bResult = is_token_type(t, T_OPEN_PAREN);
    if (bResult) {
        // should be a function decl or function body
        t = consume();
        vars = vec_new();
        result = parse_function_args(vars);
        if (result != 0) {
            return -1;
        }

        t = consume();
        bResult = is_token_type(t, T_OPEN_BRACE);
        if (bResult) {
            // should be function body
            funcDecl = func_decl_new(sym->val, var, vars);
            *declType = DECL_TYPE_FUNCTION_BODY;
            decl = funcDecl;
            return 0;

        }
        bResult = is_token_type(t, T_SEMICOLON);
        if (bResult) {
            funcDecl = func_decl_new(sym->val, var, vars);
            *declType = DECL_TYPE_FUNCTION_PROTOTYPE;
            decl = funcDecl;
            return 0;
        }
    } else {
        // should be a var delare
        t = consume();
        result = is_token_type(t, T_EQUAL);
        if (result) {
            // var declare with initial value
        }
        result = is_token_type(t, T_SEMICOLON);
        if (result) {
            // var delare;
            declType = DECL_TYPE_VAR;
            decl = var;
            return 0;
        }
    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return 0;
}

static int parse_function_args(Vector *vars)
{
    bool bResult;
    int result;
    Token* t;

    t = cur_token();
    Variable* variable = NULL;
    while (1) {
        bResult = is_token_type(t, T_CLOSE_PAREN);
        if (bResult) {
            break;
        }
        result = parse_variable(variable);
        if (result != 0) {
            return -1;
        }
        t = consume();

        if (variable != NULL) {
            vec_push_back(vars, variable);
        }

        // skip comma
        bResult = is_token_type(t, T_COMMA);
        if (bResult) {
            t = consume();
            continue;
        }
    }
    return 0;
}

static int parse_function_body(FuncDecl* func, FuncBody *funcBody) {
    bool bResult;
    int result;
    Token* t = cur_token();
    Variable* var;
    void* ast;
    Scope* scope;

    scope = scope_new();
    funcBody = func_body_new();
    while (true) {
        bResult = is_token_type(t, T_CLOSE_BRACE);
        if (bResult) {
            break;
        }

        // check return
        bResult = is_token_type(t, T_RETURN);
        if (bResult) {
            t = consume();
            result = get_return_var(t, func->ret, ast);
            if (result != 0) {
                return -1;
            }
            scope_add_stmt(ast);
            funcBody
        }
        t = consume();
    }
    return 0;
}

typedef struct {
    long val;
} AstInteger;

static AstInteger *ast_int_new(long val)
{
    AstInteger *ast = malloc(sizeof(AstInteger));
    ast->val = val;
    return ast;
}

static int get_return_var(Token *t, Variable* var, void *ast)
{
    long val;
    char* endptr;
    int result = 0;

    if (t->type == T_INTEGER) {
        val = strtol(t->val, &endptr, 10);
        ast = ast_int_new(val);
        return result;
    }

    return -1;
}

static int parse_variable(Variable *var)
{
    bool bResult;
    int result;
    Token* t;

    t = cur_token();
    bResult = is_type_of(t, s_types, s_types_len);
    if (bResult != true) {
        return -1;
    }
    bResult = is_token_type(t, T_VOID);
    if (bResult) {
        var = NULL;
        return 0;
    }

    t = consume();
    bResult = is_token_type(t, T_IDENTIFIER);
    if (bResult != true) {
        return -1;
    }

    t = consume();
    var = variable_new(0, t->type, t->val);
    return 0;
}

static bool is_type_of(Token *t, Type *types, int len)
{
    int i;
    bool bResult = false;
    for (i = 0; i < len; i++) {
        if (strcmp(t->val, types[i].name) == 0) {
            bResult = true;
        }
    }
    return bResult;
}

static bool is_token_type(Token* t, TokenType tokenType)
{
    bool bResult = false;
    if (t->type == tokenType) {
        bResult = true;
    }
    return bResult;
}
static bool is_literal_or_of(Token* t, Variable* var)
{
    bool bResult;
    switch (t->type) {
        case T_INTEGER:
            break;
        default:
            break;
    }
    return true;
}

static bool is_c_types(Token* t)
{
    int i = 0;
    int ret;
    for (i = 0; i < C_TYPES_LEN; i++)
    {
        ret = strcmp(t->val, c_types[i].name);
        if (ret == 0) {
            return true;
        }
    }
    return false;
}

static Token* cur_token()
{
    return &s_tokens[s_token_pos];
}

static Token *consume()
{
    s_token_pos++;
    if (s_token_len <= s_token_pos) {
        return NULL;
    }
    return &s_tokens[s_token_pos];
}

static int declare_specifier(VarInfo *decl)
{
    Token *t = NULL;
    decl->scope = SCOPE_STATIC;
    decl->ptr = 0;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
#if 0
    t = consume(T_IDENTIFIER);
    if (!t) {
        error_unexpected_token(T_IDENTIFIER);
        return -1;
    }
    strncpy(decl->sym, t->val, STRING_MAX);
    decl->sym[STRING_MAX -1] = '\0';

    // initialize
    if (consume(T_EQUAL)) {
        t = consume(T_INTEGER);
        if (!t) {
            error_unexpected_token(T_INTEGER);
            return -1;
        }
        decl->dt = DtInt;
    }

    t = consume(T_SEMICOLON);
    if (!t) {
        error_unexpected_token(T_SEMICOLON);
        return -1;
    }
#endif
	DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    return 0;
}

static void error_unexpected_token(TokenType expected)
{
    // TODO
}

