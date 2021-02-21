#include "variable.h"
#include "func.h"
#include <stdlib.h>
#include <string.h>

FuncDecl *func_decl_new(char *name, Type *retType, Vector *args)
{
    FuncDecl* funcDecl = malloc(sizeof(FuncDecl));
    strcpy(funcDecl->name, name);
    funcDecl->retType = retType;
    funcDecl->args = args;
    return funcDecl;
}

FuncBody* func_body_new()
{
    FuncBody* funcBody = malloc(sizeof(FuncBody));
    funcBody->scope = scope_new();
    return funcBody;
}