#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "type.h"
#include "debug.h"

Type* type_new(char* name, int size)
{
    // TODO must set hash value
    Type *t = malloc(sizeof(Type));
    strcpy(t->name, name);
    t->size = size;
    return t;
}
bool is_same_type(Type* ty1, Type* ty2)
{
    return (strcmp(ty1->name, ty2->name) == 0);
}
