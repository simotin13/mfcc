#ifndef _SCOPE_H_
#define _SCOPE_H_
#include "type.h"
#include "vector.h"

typedef struct _Scope{
    Scope *nest;
    Vector *vars;
    Vector *stmts; 
} Scope;

Scope scope_new();

#endif  // _SCOPE_H_