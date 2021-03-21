#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lex.h"
#include "ast.h"
#include "type.h"
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
static int parse_function_body(Vector *globalVars, FuncDecl* funcDecl, FuncBody* funcBody);
static int parse_local_variables(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody);
static int parse_variable(Variable** var);
static Integer* ast_int_new(long val);
static int parse_return_stmt(Vector *globalVars, FuncDecl* funcDecl, FuncBody* funcBody, Token* t, StmtReturn** stmtReturn);
static int parse_assign_stmt(Vector *globalVars, FuncDecl *funcDecl, FuncBody *funcBody, Token *tLhs, AstNode **nodeTop);
static int parse_expression(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, AstNode** nodeTop);

static int find_var(Vector* vars, char* name);
static Token* cur_token();
static bool is_type_of(Token *t, Vector *types, Type **type);
static bool is_token_type(Token* t, TokenType tokenType);
static bool is_term(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, Token* t, Term** term);
static bool is_operator(Token* t, OperationType *op);

static Token *consume(void);

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
    Vector* globalVars = NULL;
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
            result = parse_function_body(globalVars, funcDecl, funcBody);
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
    Vector* vars;
    FuncDecl *funcDecl;
    Type* type;
    vars = vec_new();
    StorageClass class = ClassLocal;

    t = cur_token();
    bResult = is_token_type(t, T_STATIC);
    if (bResult) {
        class = ClassStatic;
        t = consume();
    } else {
        bResult = is_token_type(t, T_EXTERN);
        if (bResult) {
            class = ClassExtern;
            t = consume();
        }
    }

    bResult = is_type_of(t, s_types, &type);
    if (bResult != true) {
        return -1;
    }

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
            funcDecl = func_decl_new(sym->val, class, type, vars);
            *declType = DECL_TYPE_FUNCTION_BODY;
            *decl = funcDecl;
            return 0;
        }
        bResult = is_token_type(t, T_SEMICOLON);
        if (bResult) {
            funcDecl = func_decl_new(sym->val, class, type, vars);
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
            *decl = variable_new(sym->val, class, type, 0);
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
        result = parse_variable(&variable);
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

static int parse_function_body(Vector *globalVars, FuncDecl* funcDecl, FuncBody *funcBody)
{
    bool bResult;
    int ret;
    Token* t = cur_token();
    Stmt* stmt;
    StmtReturn* stmtReturn;

    while (true) {
        bResult = is_token_type(t, T_CLOSE_BRACE);
        if (bResult) {
            consume();
            break;
        }

        // check local variable
        ret = parse_local_variables(globalVars, funcDecl, funcBody);
        if (ret < 0) {
            return -1;
        }

        // check return
        t = cur_token();
        bResult = is_token_type(t, T_RETURN);
        if (bResult) {
            t = consume();
            ret = parse_return_stmt(globalVars, funcDecl, funcBody, t, &stmtReturn);
            if (ret != 0) {
                return -1;
            }
            stmt = stmt_new(STMT_TYPE_RETURN, stmtReturn);
            scope_add_stmt(funcBody->scope, stmt);
        }
        t = consume();
    }
    return 0;
}
static int parse_local_variables(Vector *globalVars, FuncDecl *funcDecl, FuncBody* funcBody)
{
    Token* tLhs;
    Token* t;
    Type* type;
    bool bResult;
    Variable *var;
    AstNode* node;
    int ret;

    while (true) {
        // check exist variable declare
        tLhs = cur_token();
        bResult = is_type_of(tLhs, s_types, &type);
        if (bResult != true) {
            return funcBody->scope->vars->size;
        }

        ret = parse_variable(&var);
        if (ret != 0) {
            return -1;
        }
 
        t = consume();
        bResult = is_token_type(t, T_EQUAL);
        if (bResult) {
            // assign
            t = consume();
            ret = parse_assign_stmt(globalVars, funcDecl, funcBody, tLhs, &node);
            if (ret != 0) {
                return -1;
            }
            var->initialAssign = node;
        } else {
            // local variable declare
            bResult = is_token_type(t, T_SEMICOLON);
            if (bResult != true) {
                return -1;
            }
        }
        vec_push(funcBody->scope->vars, var);

        // next token
        consume();
    }

    return 0;
}

static Integer *ast_int_new(long val)
{
    Integer *ast = malloc(sizeof(Integer));
    ast->val = val;
    return ast;
}

static int parse_assign_stmt(Vector *globalVars, FuncDecl *funcDecl, FuncBody *funcBody, Token* tLhs, AstNode** nodeTop)
{
    int ret;
    ret = parse_expression(globalVars, funcDecl, funcBody, nodeTop);
    return ret;
}

typedef enum {
    NONE,
    WAIT_TERM,
    WAIT_OPERATOR,
} ParseExpStatus;

static int parse_expression(Vector *globalVars, FuncDecl* funcDecl, FuncBody* funcBody, AstNode **node)
{
    Token* t;
    AstBinary* astBin;
    Term* term = NULL;
    AstNode *tmp = NULL;
    AstNode *lhs = NULL;
    AstNode *rhs = NULL;
    OperationType opStack[32] = { 0 };
    OperationType op;
    AstNode* nodeStack[32] = { 0 };
    int opPos = 0;
    int nodePos = 0;
    int nest = 0;
    while (true) {
        t = cur_token();
        if (is_token_type(t, T_OPEN_PAREN)) {
            nest++;
            consume();
            continue;
        }
        if (is_token_type(t, T_CLOSE_PAREN)) {
            nest--;
            if (nest < 0) {
                return -1;
            }
            if (0 < opPos) {
                opPos--;
                op = opStack[opPos];

                nodePos--;
                rhs = nodeStack[nodePos];

                nodePos--;
                lhs = nodeStack[nodePos];

                astBin = ast_binary_new(op, lhs, rhs);
                nodeStack[nodePos] = node_new(NODE_BINARY, astBin);
                nodePos++;
            }
            consume();
            continue;
        }
        if (is_operator(t, &op)) {
            opStack[opPos] = op;
            opPos++;
            consume();
            continue;
        }
        if (is_term(globalVars, funcDecl, funcBody, t, &term)) {
            nodeStack[nodePos] = node_new(NODE_TERM, term);
            nodePos++;
            consume();
            continue;
        }

        if (is_token_type(t, T_SEMICOLON)) {
            break;
        }
    }

    while (0 < opPos) {
        opPos--;
        op = opStack[opPos];
        nodePos--;
        tmp = nodeStack[nodePos];
        rhs = node_new(tmp->type, tmp->entry);

        nodePos--;
        tmp = nodeStack[nodePos];
        lhs = node_new(tmp->type, tmp->entry);

        astBin = ast_binary_new(op, lhs, rhs);
        nodeStack[nodePos] = node_new(NODE_BINARY, astBin);
        nodePos++;
    }

    *node = nodeStack[0];
    return 0;
}

static bool is_term(Vector* globalVars, FuncDecl *funcDecl, FuncBody *funcBody, Token* t, Term **term)
{
    long val;
    char* endptr;
    bool bResult;
    Variable* var = NULL;
    int idx;
    Type* ty;
    bResult = is_token_type(t, T_INTEGER);
    if (bResult) {

        val = strtol(t->val, &endptr, 10);
        ty = (Type*)&(c_types[C_TYPES_IDX_INT]);
        *term = term_new(TermLiteral, ty, ast_int_new(val));
        return true;
    }
    bResult = is_token_type(t, T_IDENTIFIER);
    if (bResult) {
        // search args
        idx = find_var(funcDecl->args, t->val);
        if (0 <= idx) {
            var = funcDecl->args->data[idx];
            *term = term_new(TermVariable, var->ty, var);
            return true;
        }

        // return varible
        idx = find_var(funcBody->scope->vars, t->val);
        if (0 <= idx) {
            var = funcDecl->args->data[idx];
            *term = term_new(TermVariable, var->ty, var);
            return true;
        }
    }

    return false;
}
static bool is_operator(Token* t, OperationType *op)
{
    bool bResult;
    bResult = is_token_type(t, T_PLUS);
    if (bResult) {
        *op = OPERATION_ADD;
        return true;
    }
    bResult = is_token_type(t, T_MINUS);
    if (bResult) {
        *op = OPERATION_SUB;
        return true;
    }
    bResult = is_token_type(t, T_ASTER);
    if (bResult) {
        *op = OPERATION_MUL;
        return true;
    }
    bResult = is_token_type(t, T_SLASH);
    if (bResult) {
        *op = OPERATION_DIV;
        return true;
    }

    return false;
}

static int parse_return_stmt(Vector *globalVars, FuncDecl *funcDecl, FuncBody *funcBody, Token *t, StmtReturn **stmtReturn)
{
    int result = 0;
    AstNode* node;
    result = parse_expression(globalVars, funcDecl, funcBody, &node);
    *stmtReturn = stmt_new_stmt_return(node);
    return result;
}

static int find_var(Vector* vars, char* name) {
    int i;
    Variable* var;
    for (i = 0; i < vars->size; i++) {
        var = vars->data[i];
        if (strcmp(var->name, name) == 0) {
            return i;
        }
    }
    return -1;
}
static int parse_variable(Variable **var)
{
    bool bResult;
    Token* t;
    Type* type;
    StorageClass class = ClassLocal;

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

    *var = variable_new(t->val, class, type, 0);
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

