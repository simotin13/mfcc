#ifndef _FUNC_H_
#define _FUNC_H_
#include "type.h"
#include "vector.h"
#include "ast.h"
#include "variable.h"

extern FuncDecl* func_decl_new(char *name, StorageClass class, Type* retType, Vector *args);
extern FuncBody* func_body_new();

#endif  // _FUNC_H_