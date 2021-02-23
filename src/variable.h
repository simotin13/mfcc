#ifndef _VARIABLE_H_
#define _VARIABLE_H_
#include "type.h"

typedef struct {
    StorageClass class;
    int pointer_level;
    Type *ty;
    char name[256];
    void* iVal;
} Variable;

extern Variable* variable_new(char* name, StorageClass class, Type* ty, int pointer_level);

#endif  // _VARIABLE_H_