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

// ============================================================================
// prototype functions
// ============================================================================
static int parse_toplevel(Program* program);
static int parse_declare_specifier(DeclType *declType, void **decl);
static int parse_function_args(Vector *args);
static int parse_function_body(FuncDecl* func, FuncBody* funcBody);
static int parse_variable(Variable* var);
static Integer* ast_int_new(long val);
static int get_return_stmt(Token* t, Variable* var, StmtReturn** stmtReturn);

static Token* cur_token();
static bool is_type_of(Token *t, Vector *types, Type **type);
static bool is_token_type(Token* t, TokenType tokenType);
static Token *consume(void);
static void error_unexpected_token(TokenType expected);

// token info
static Token* s_tokens = NULL;
static int s_token_pos = 0;
static int s_token_len = 0;
static Vector* s_types;

int parse_tokens(Vector *tokens, Vector *dataTypes, Program *program)
{
    int i;
    int result;
    Token* token;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
        
    // copy tokens
    s_tokens = malloc(sizeof(Token) * tokens->size);
    for (i = 0; i < tokens->size; i++) {
        token = (Token *)tokens->data[i];
        memcpy(&s_tokens[i], token, sizeof(Token));
    }

    s_token_pos = 0;
    s_token_len = tokens->size;
    s_types = dataTypes;
    result = parse_toplevel(program);

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return result;
}


static int parse_toplevel(Program* program)
{
    int result = 0;
    DeclType declType;
    Func* func = NULL;
    FuncDecl* funcDecl = NULL;
    FuncBody* funcBody = NULL;
    void* decl = NULL;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    while(s_token_pos < s_token_len) {
        result = parse_declare_specifier(&declType, &decl);
        if (result != 0) {
            return -1;
        }
        if (declType == DECL_TYPE_VAR) {
            // TODO
        } else if (declType == DECL_TYPE_FUNCTION_PROTOTYPE) {
            // TODO
        } else if (declType == DECL_TYPE_FUNCTION_BODY) {
            funcDecl = decl;
            funcBody = func_body_new();
            result = parse_function_body(funcDecl, funcBody);
            if (result != 0) {
                return -1;
            }
            func = func_new(funcDecl, funcBody);
            vec_push(program->funcs, func);
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
    return 0;
}

static int parse_declare_specifier(DeclType *declType, void **decl) {
    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);
    bool bResult;
    int result;
    Token* t;
    Token* sym;
    Variable* var;
    Vector* vars;
    FuncDecl *funcDecl;
    Type* type;
    vars = vec_new();

    t = cur_token();
    bResult = is_type_of(t, s_types, &type);
    if (bResult != true) {
        return -1;
    }
    var = variable_new(type, 0);

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
            consume();
            funcDecl = func_decl_new(sym->val, var, vars);
            *declType = DECL_TYPE_FUNCTION_BODY;
            *decl = funcDecl;
            return 0;

        }
        bResult = is_token_type(t, T_SEMICOLON);
        if (bResult) {
            funcDecl = func_decl_new(sym->val, var, vars);
            *declType = DECL_TYPE_FUNCTION_PROTOTYPE;
            *decl = funcDecl;
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
            *declType = DECL_TYPE_VAR;
            *decl = var;
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
    while (true) {
        bResult = is_token_type(t, T_CLOSE_PAREN);
        if (bResult) {
            break;
        }
        result = parse_variable(variable);
        if (result != 0) {
            return -1;
        }

        if (variable != NULL) {
            vec_push(vars, variable);
        }

        // skip comma
        t = consume();
        bResult = is_token_type(t, T_COMMA);
        if (bResult) {
            t = consume();
            continue;
        }
    }
    return 0;
}

static int parse_function_body(FuncDecl* func, FuncBody *funcBody)
{
    bool bResult;
    int result;
    Token* t = cur_token();
    Stmt* stmt;
    StmtReturn* stmtReturn;

    while (true) {
        bResult = is_token_type(t, T_CLOSE_BRACE);
        if (bResult) {
            consume();
            break;
        }

        // check return
        bResult = is_token_type(t, T_RETURN);
        if (bResult) {
            t = consume();
            result = get_return_stmt(t, func->ret, &stmtReturn);
            if (result != 0) {
                return -1;
            }
            stmt = stmt_new(STMT_TYPE_RETURN, stmtReturn);
            scope_add_stmt(funcBody->scope, stmt);
        }
        t = consume();
    }
    return 0;
}

static Integer *ast_int_new(long val)
{
    Integer *ast = malloc(sizeof(Integer));
    ast->val = val;
    return ast;
}

static int get_return_stmt(Token *t, Variable* var, StmtReturn **stmtReturn)
{
    long val;
    char* endptr;
    int result = 0;
    Integer* astInt;
    Token *tmp;
    bool bResult;
    if (t->type == T_INTEGER) {
        tmp = consume();
        bResult = is_token_type(tmp, T_SEMICOLON);
        if (bResult) {
            val = strtol(t->val, &endptr, 10);
            astInt = ast_int_new(val);
            *stmtReturn = stmt_new_stmt_return(t, astInt);
            return 0;
        }
    }

    return -1;
}

static int parse_variable(Variable *var)
{
    bool bResult;
    Token* t;
    Type* type;

    t = cur_token();
    bResult = is_type_of(t, s_types, &type);
    if (bResult != true) {
        return -1;
    }

    // void doesn't hava identifier
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

    var = variable_new(type, 0);
    return 0;
}

static bool is_type_of(Token *t, Vector *types, Type **type)
{
    int i;
    bool bResult = false;
    Type* ty;
    for (i = 0; i < types->size; i++) {
        ty = (Type *)types->data[i];
        if (strcmp(t->val, ty->name) == 0) {
            *type = types->data[i];
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

static void error_unexpected_token(TokenType expected)
{
    // TODO
}

