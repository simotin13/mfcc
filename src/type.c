#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "debug.h"

Type *type_new(char *name)
{
    Type *t = malloc(sizeof(Type));
    strcpy(t->name, name);
    return t;
}
