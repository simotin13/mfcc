#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "type.h"
#include "hash.h"
#include "debug.h"

const Type c_types[] =
{
    {   0,              0,      "void"      },
    {   1,   sizeof(char),      "char"      },
    {   2,   sizeof(short),     "short"     },
    {   3,   sizeof(int),       "int"       },
    {   4,   sizeof(long),      "long"      },
    {   5,   sizeof(float),     "float"     },
    {   6,   sizeof(double),    "double"    },
};

Type* type_new(char* name, int size)
{
    // TODO must set hash value
    Type *t = malloc(sizeof(Type));
    strcpy(t->name, name);
    t->size = size;
    t->hash = hash(name);
    return t;
}
bool is_same_type(Type* ty1, Type* ty2)
{
    return (strcmp(ty1->name, ty2->name) == 0);
}
