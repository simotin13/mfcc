#ifndef _TYPE_H_
#define _TYPE_H_
#include <stdbool.h>

#define C_TYPES_LEN     (7)

typedef struct {
    int hash;
    int size;
    char name[256];
} Type;

Type* type_new(char* name, int size);
extern bool is_same_type(Type* ty1, Type* ty2);

#endif  // _TYPE_H_