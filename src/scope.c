#include "variable.h"
#include "scope.h"
#include <stdlib.h>
#include <string.h>

Scope* scope_new(char *name, Variable *ret, Vector *args)
{
    Scope* scope = malloc(sizeof(Scope));
    scope->nest = NULL;
    scope->vars = vec_new();
    scope->stmts = vec_new();
    return scope;
}
