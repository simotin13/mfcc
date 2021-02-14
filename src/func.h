#ifndef _FUNC_H_
#define _FUNC_H_
#include "type.h"
#include "vector.h"
#include "ast.h"
#include "variable.h"

extern FuncDecl* func_decl_new(char *name, Variable *ret, Vector *args);
extern FuncBody* func_body_new();

#endif  // _FUNC_H_