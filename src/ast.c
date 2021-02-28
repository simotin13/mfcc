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

StmtReturn* stmt_new_stmt_return(Expression *exp)
{
    StmtReturn* stmt = malloc(sizeof(StmtReturn));
    stmt->exp = exp;
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

Expression* exp_new()
{
    Expression* exp;
    exp = malloc(sizeof(Expression));
    exp->operations = vec_new();
    exp->terms = vec_new();
    return exp;
}

void exp_add_op(Expression* exp, TokenType ty)
{
    TokenType* opTy = malloc(sizeof(TokenType));
    *opTy = ty;
    vec_push(exp->operations, opTy);
}
void exp_add_term(Expression* exp, Term* term)
{
    vec_push(exp->terms, term);
}

Variable* variable_new(char* name, StorageClass class, Type* ty, int pointer_level)
{
    Variable* var = NULL;
    var = malloc(sizeof(Variable));
    strcpy(var->name, name);
    var->class = class;
    var->pointer_level = pointer_level;
    var->ty = ty;
    var->initialAssignExp = NULL;
    return var;
}
