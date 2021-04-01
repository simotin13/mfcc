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
AssignStmt* assign_stmt_new(AstNode *node) {
    AssignStmt* stmt = malloc(sizeof(AssignStmt));
    stmt->node = node;
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

ReturnStmt* stmt_new_stmt_return(AstNode *node)
{
    ReturnStmt* stmt = malloc(sizeof(ReturnStmt));
    stmt->node = node;
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
    term->termType = type;
    term->ty = ty;
    term->ast = ast;
    return term;
}
AstNode* node_new(NodeType type, void *entry) {
    AstNode* node = malloc(sizeof(AstNode));
    node->type = type;
    node->entry = entry;
    return node;
}
AstBinary* ast_binary_new(OperationType op, AstNode* lhs, AstNode* rhs)
{
    AstBinary* ast = malloc(sizeof(AstBinary));
    ast->op = op;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

AstBinary* ast_binary_set_elements(AstBinary *ast, OperationType op, AstNode *lhs, AstNode *rhs)
{
    ast->op = op;
    ast->lhs = lhs;
    ast->rhs = rhs;
    return ast;
}

Variable* variable_new(char* name, StorageClass class, Type* ty, int pointer_level)
{
    Variable* var = NULL;
    var = malloc(sizeof(Variable));
    strcpy(var->name, name);
    var->class = class;
    var->pointer_level = pointer_level;
    var->ty = ty;
    var->initialAssign = NULL;
    return var;
}
