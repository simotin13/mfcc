#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

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
static int parse_declare(DeclType *declType, void **decl);
static int parse_function_args(Vector *args);
static int parse_function_body(Vector *globalVars, FuncDecl* funcDecl, FuncBody* funcBody);
static int parse_local_variables(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody);
static int parse_assign_or_function_call(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody);
static int parse_function_call_args(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, Vector *args);
static int parse_variable(Variable** var);
static Integer* ast_int_new(long val);
static int parse_return_stmt(Vector *globalVars, FuncDecl* funcDecl, FuncBody* funcBody, ReturnStmt** stmtReturn);
static int parse_arg_expression(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, AstNode** node);
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
    FuncDecl* funcDecl = NULL;
    FuncBody* funcBody = NULL;
    void* decl = NULL;

    DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    while(s_token_pos < s_token_len) {
        result = parse_declare(&declType, &decl);
        if (result != 0) {
            return -1;
        }
        if (declType == DECL_TYPE_VAR) {
			vec_push(program->vars, decl);
        } else if (declType == DECL_TYPE_FUNCTION_PROTOTYPE) {
            // TODO
        } else if (declType == DECL_TYPE_FUNCTION_BODY) {
            funcDecl = decl;
            funcBody = func_body_new();
            result = parse_function_body(program->vars, funcDecl, funcBody);
            if (result != 0) {
                return -1;
            }
            func = func_new(funcDecl, funcBody);
            vec_push(program->funcs, func);
        }
    }

    DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);

    return 0;
}
static int parse_declare(DeclType *declType, void **decl) {
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
		*declType = DECL_TYPE_VAR;
		Variable *var = variable_new(sym->val, class, type, 0);

		result = is_token_type(t, T_EQUAL);
        if (result) {
			// global variable declare with assign
			AstNode* node;
			consume();

			// var declare with initial value
			// TODO shoulde be constant value but call parse_declare_specifier..
			int ret = parse_expression(NULL, NULL, NULL, &node);
			if (ret != 0) {
				return -1;
			}

            var->initialAssign = node;
            t = cur_token();
		}
		result = is_token_type(t, T_SEMICOLON);
		if (result != true) {
			return -1;
		}
		consume();

		*decl = var;
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
    Token* t = NULL;
    Stmt* stmt;
    ReturnStmt* stmtReturn;

    while (true) {
        t = cur_token();
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

        // check assign stmt

        // check function call
        ret = parse_assign_or_function_call(globalVars, funcDecl, funcBody);
        if (ret < 0) {
            return -1;
        }

        // check return
        t = cur_token();
        bResult = is_token_type(t, T_RETURN);
        if (bResult) {
            t = consume();
            ret = parse_return_stmt(globalVars, funcDecl, funcBody, &stmtReturn);
            if (ret != 0) {
                return -1;
            }
            stmt = stmt_new(STMT_RETURN, stmtReturn);
            scope_add_stmt(funcBody->scope, stmt);
            consume();
        }
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
            consume();

            // assign
            ret = parse_expression(globalVars, funcDecl, funcBody, &node);
            if (ret != 0) {
                return -1;
            }

            t = cur_token();
            bResult = is_token_type(t, T_SEMICOLON);
            if (bResult != true) {
                return -1;
            }
            consume();
            var->initialAssign = node;
        } else {
            // local variable declare
            bResult = is_token_type(t, T_SEMICOLON);
            if (bResult != true) {
                return -1;
            }
            consume();
        }
        vec_push(funcBody->scope->vars, var);
    }

    return 0;
}

static int parse_assign_or_function_call(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody)
{
    Token* identifier;
    Token* t;
    bool bResult;
    AstNode* node;
    int ret;
    AssignStmt* assignStmt;
    AstFuncCall* funcCall;
    FuncCallStmt* funcCallStmt;
    Variable* lhs = NULL;
    Vector* args = NULL;
    int idx;

    identifier = cur_token();

    while (true) {
        bResult = is_token_type(identifier, T_IDENTIFIER);
        if (bResult != true) {
            // not function call
            return 0;
        }

        t = consume();
        bResult = is_token_type(t, T_OPEN_PAREN);
        if (bResult) {
            // must be function call
            args = vec_new();
            ret = parse_function_call_args(globalVars, funcDecl, funcBody, args);
            if (ret != 0) {
                return -1;
            }

            funcCall = fanc_call_new(identifier->val, args);
            funcCallStmt = func_call_stmt_new(funcCall);
            vec_push(funcBody->scope->stmts, stmt_new(STMT_FUNC_CALL, funcCallStmt));
            
            consume();
            return 0;
        }
        bResult = is_token_type(t, T_EQUAL);

        if (bResult) {
            consume();
            ret = parse_expression(globalVars, funcDecl, funcBody, &node);
            if (ret != 0) {
                return -1;
            }
            t = cur_token();
            bResult = is_token_type(t, T_SEMICOLON);
            if (bResult != true) {
                return -1;
            }

            consume();

            // search local variable first
            idx = find_var(funcBody->scope->vars, identifier->val);
            if (0 <= idx) {
                lhs = (Variable *)funcBody->scope->vars->data[idx];
                assignStmt = assign_stmt_new(lhs, node);
                vec_push(funcBody->scope->stmts, stmt_new(STMT_ASSIGN, assignStmt));
                break;
            }

            idx = find_var(globalVars, identifier->val);
            if (0 <= idx) {
                lhs = (Variable*)globalVars->data[idx];
                assignStmt = assign_stmt_new(lhs, node);
                vec_push(funcBody->scope->stmts, stmt_new(STMT_ASSIGN, assignStmt));
                break;
            }
        }
        return -1;
    }

    return 0;
}

static int parse_function_call_args(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, Vector *args) {
    Token* t;
    int result;
    bool bResult;
    AstNode* arg;

    while (true) {
        t = cur_token();
        result = parse_arg_expression(globalVars, funcDecl, funcBody, &arg);
        if (result != 0) {
            return -1;
        }

        vec_push(args, arg);

        t = cur_token();
        bResult = is_token_type(t, T_COMMA);
        if (bResult) {
            consume();
            continue;
        }

        bResult = is_token_type(t, T_CLOSE_PAREN);
        if (bResult) {
            consume();
            break;
        }
        return -1;
    }
    return 0;
}

static Integer *ast_int_new(long val)
{
    Integer *ast = malloc(sizeof(Integer));
    ast->val = val;
    return ast;
}
static AstString* ast_string_new(char *str)
{
    AstString* ast = malloc(sizeof(AstString));
    strcpy(ast->val, str);
    // TODO size check
    return ast;
}

typedef enum {
    NONE,
    WAIT_TERM,
    WAIT_OPERATOR,
} ParseExpStatus;

static int parse_arg_expression(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, AstNode** node)
{
    int ret;
    Token* identifier;
    Token* t;
    AstBinary* astBin;
    Term* term = NULL;
    AstNode* tmp = NULL;
    AstNode* lhs = NULL;
    AstNode* rhs = NULL;
    AstFuncCall* funcCall = NULL;
    OperationType op;
    OperationType opStack[32] = { 0 };
    AstNode* nodeStack[32] = { 0 };
    Vector* args;
    int opPos = 0;
    int nodePos = 0;
    int nest = 0;
    while (true) {
        t = cur_token();

        if (is_token_type(t, T_IDENTIFIER)) {
            identifier = t;
            t = consume();
            if (is_token_type(t, T_OPEN_PAREN)) {
                // must be function call
                consume();
                args = vec_new();
                ret = parse_function_call_args(globalVars, funcDecl, funcBody, args);
                if (ret != 0) {
                    return -1;
                }

                funcCall = fanc_call_new(identifier->val, args);
                nodeStack[nodePos] = node_new(NODE_FUNC_CALL, funcCall);
                nodePos++;
            }
            return -1;
        }

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

            t = consume();
            if (is_token_type(t, T_COMMA)) {
                break;
            }
            if (is_token_type(t, T_CLOSE_PAREN)) {
                break;
            }
            return -1;
        }

        return -1;
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

static int parse_expression(Vector* globalVars, FuncDecl* funcDecl, FuncBody* funcBody, AstNode** node)
{
    int ret;
    Token* identifier;
    Token* t;
    AstBinary* astBin;
    Term* term = NULL;
    AstNode* tmp = NULL;
    AstNode* lhs = NULL;
    AstNode* rhs = NULL;
    AstFuncCall* funcCall = NULL;
    Vector* args = NULL;
    OperationType opStack[32] = { 0 };
    OperationType op;
    AstNode* nodeStack[32] = { 0 };
    int opPos = 0;
    int nodePos = 0;
    int nest = 0;
    while (true) {
        t = cur_token();

        if (is_token_type(t, T_IDENTIFIER)) {
            identifier = t;
            t = consume();
            if (is_token_type(t, T_OPEN_PAREN)) {
                consume();

                // must be function call
                args = vec_new();
                ret = parse_function_call_args(globalVars, funcDecl, funcBody, args);
                if (ret != 0) {
                    return -1;
                }

                funcCall = fanc_call_new(identifier->val, args);
                nodeStack[nodePos] = node_new(NODE_FUNC_CALL, funcCall);
                nodePos++;
                continue;
            } else {
                // must be variable
                if (is_term(globalVars, funcDecl, funcBody, identifier, &term)) {
                    nodeStack[nodePos] = node_new(NODE_TERM, term);
                    nodePos++;
                } else {
                    assert(0);
                    return -1;
                }
            }
        }

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
        break;
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
    Type* ty = NULL;
    bResult = is_token_type(t, T_DEC_NUMBER_LITERAL);
    if (bResult) {
        val = strtol(t->val, &endptr, 10);
        ty = (Type*)&(c_types[C_TYPES_IDX_INT]);
        *term = term_new(TermIntLiteral, ty, ast_int_new(val));
        return true;
    }
    bResult = is_token_type(t, T_STRING_LITERAL);
    if (bResult) {
        Type* ty = NULL;
        *term = term_new(TermStringLiteral, ty, ast_string_new(t->val));
        return true;
    }
    bResult = is_token_type(t, T_IDENTIFIER);
    if (bResult) {
        // search args
        idx = find_var(funcDecl->args, t->val);
        if (0 <= idx) {
            var = funcDecl->args->data[idx];
            *term = term_new(TermArgVariable, var->ty, var);
            return true;
        }

        // local varible
        idx = find_var(funcBody->scope->vars, t->val);
        if (0 <= idx) {
            var = funcBody->scope->vars->data[idx];
            *term = term_new(TermLocalVariable, var->ty, var);
            return true;
        }

		// search global vars
		idx = find_var(globalVars, t->val);
		if (0 <= idx) {
			var = globalVars->data[idx];
			*term = term_new(TermGlobalVariable, var->ty, var);
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

static int parse_return_stmt(Vector *globalVars, FuncDecl *funcDecl, FuncBody *funcBody, ReturnStmt **stmtReturn)
{
    int result = 0;
    bool bResult;
    AstNode* node;
    Token* t;
    t = cur_token();
    result = parse_expression(globalVars, funcDecl, funcBody, &node);
    if (result != 0) {
        return -1;
    }
    t = cur_token();
    bResult = is_token_type(t, T_SEMICOLON);
    if (bResult != true) {
        return -1;
    }
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

    // void doesn't have identifier
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
