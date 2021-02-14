#ifndef _AST_H_
#define _AST_H_

#include "lex.h"
#include "variable.h"

// ============================================================================
// struct define
// ============================================================================
typedef struct {
    Vector *vars;
    Vector *funcs;
} Program;

typedef struct {
    char name[256];
    Variable* ret;
    Vector* args;
} FuncDecl;

typedef struct {
    Vector* vars;
    Vector* stmts;
} Scope;

typedef struct {
    Scope* scope;
} FuncBody;

typedef struct {
    FuncDecl *decl;
    FuncBody *body;
} Func;

typedef enum {
    STMT_TYPE_RETURN,
} StatementType;

typedef struct {
    long val;
} Integer;

typedef struct {
    Token* t;
    void* val;
}StmtReturn;

typedef struct {
    StatementType type;
    void* ast;
}Stmt;

extern Program* program_new();
extern Stmt *stmt_new(StatementType type, void* ast);
extern Func* func_new(FuncDecl* decl, FuncBody* body);
extern Scope* scope_new(void);
extern void scope_add_stmt(Scope* scope, Stmt* stmt);
extern StmtReturn* stmt_new_stmt_return(Token* t, void* val);
#endif // _AST_H_