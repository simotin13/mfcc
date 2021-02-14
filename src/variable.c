#include "variable.h"
#include <stdlib.h>
#include <string.h>

Variable *variable_new(char* name, Type* ty, int pointer_level)
{
    Variable* var = NULL;
    var = malloc(sizeof(Variable));
    var->pointer_level = pointer_level;
    var->ty = ty;
    strcpy(var->name, name);
    return var;
}
