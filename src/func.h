#ifndef _FUNC_H_
#define _FUNC_H_
#include "type.h"
#include "vector.h"
#include "variable.h"
#include "scope.h"

typedef struct {
    char name[256];
    Variable *ret;
    Vector *args;
} FuncDecl;

typedef struct {
    Scope* scope;
} FuncBody;

extern FuncDecl *func_decl_new(char *name, Variable *ret, Vector *args);
extern FuncBody* func_body_new();

#endif  // _FUNC_H_