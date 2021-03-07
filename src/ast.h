#ifndef _AST_H_
#define _AST_H_

#include "lex.h"
#include "type.h"

// ============================================================================
// struct define
// ============================================================================
typedef enum {
    ExpInteger,
} ExpType;

typedef enum {
    TermLiteral,
    TermVariable
} TermType;

// ============================================================================
// struct define
// ============================================================================
typedef struct {
    Vector *vars;
    Vector *funcs;
} Program;

typedef struct {
    StorageClass class;
    char name[256];
    Type* retType;
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
    int val;
} Integer;

typedef struct {
    TermType type;
    Type* ty;
    void* ast;
} Term;

typedef struct {
    Vector *operations;
    Vector *terms;
} Expression;

typedef struct {
    StatementType type;
    void* ast;
} Stmt;

typedef struct {
    Expression* exp;
} StmtReturn;

typedef struct {
    StorageClass class;
    int pointer_level;
    Type *ty;
    char name[256];
    Expression *initialAssignExp;
} Variable;

typedef struct {
    Variable* lhs;
    Expression* rhs;
} AssignStmt;

extern Program* program_new();
extern Stmt *stmt_new(StatementType type, void* ast);
extern Func* func_new(FuncDecl* decl, FuncBody* body);
extern Scope* scope_new(void);
extern void scope_add_stmt(Scope* scope, Stmt* stmt);
extern StmtReturn* stmt_new_stmt_return(Expression* val);
extern Expression* exp_new();
extern void exp_add_op(Expression *exp, TokenType op);
extern void exp_add_term(Expression *exp, Term* term);
extern Term* term_new(TermType type, Type* ty, void* ast);
extern Variable* variable_new(char* name, StorageClass class, Type* ty, int pointer_level);

#endif // _AST_H_