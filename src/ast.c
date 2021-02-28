#include "variable.h"
#include "ast.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

Scope* scope_new(void)
{
    Scope* scope = malloc(sizeof(Scope));
    scope->vars = vec_new();
    scope->stmts = vec_new();
    return scope;
}
Stmt* stmt_new(StatementType type, void* ast)
{
    Stmt* stmt = malloc(sizeof(Stmt));
    stmt->type = type;
    stmt->ast = ast;
    return stmt;
}

void scope_add_stmt(Scope* scope, Stmt* stmt)
{
    vec_push(scope->stmts, stmt);
}

Func* func_new(FuncDecl* decl, FuncBody* body)
{
    Func* func = malloc(sizeof(Func));
    func->decl = decl;
    func->body = body;
    return func;
}

StmtReturn* stmt_new_stmt_return(Token* t, void* val)
{
    StmtReturn* stmt = malloc(sizeof(StmtReturn));
    stmt->t = t;
    stmt->val = val;
    return stmt;
}

Program *program_new()
{
    Program* program = malloc(sizeof(Program));
    program->funcs = vec_new();
    program->vars = vec_new();
    return program;
}

Term *term_new(TermType type, Type *ty, void *ast)
{
    Term* term = malloc(sizeof(Term));
    term->type = type;
    term->ty = ty;
    term->ast = ast;
    return term;
}

Operator* op_new(TokenType ty, Term* lhs, Term* rhs)
{
    Operator* op;
    op = malloc(sizeof(Operator));
    op->ty = ty;
    op->lhs = lhs;
    op->rhs = rhs;
    return op;
}
