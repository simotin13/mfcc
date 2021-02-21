#ifndef _TYPE_H_
#define _TYPE_H_

#define C_TYPES_LEN     (7)

typedef struct {
    int hash;
    int size;
    char name[256];
} Type;

Type* type_new(char* name, int size);

#endif  // _TYPE_H_