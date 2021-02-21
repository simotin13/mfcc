#include <string.h>
#include <stdlib.h>
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
