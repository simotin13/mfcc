#ifndef _VARIABLE_H_
#define _VARIABLE_H_
#include "type.h"

typedef struct {
    int pointer_level;
    Type *ty;
    char name[256];
} Variable;

extern Variable *variable_new(char* name, Type* ty, int pointer);

#endif  // _VARIABLE_H_