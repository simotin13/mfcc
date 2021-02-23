#include "variable.h"
#include <stdlib.h>
#include <string.h>

Variable *variable_new(char *name, StorageClass class, Type* ty, int pointer_level)
{
    Variable* var = NULL;
    var = malloc(sizeof(Variable));
    strcpy(var->name, name);
    var->class = class;
    var->pointer_level = pointer_level;
    var->ty = ty;
    var->iVal = NULL;
    return var;
}
