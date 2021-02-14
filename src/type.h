#ifndef _TYPE_H_
#define _TYPE_H_

typedef struct {
    int hash;
    char name[256];
} Type;

extern Type *type_new(char *name);

#endif  // _TYPE_H_