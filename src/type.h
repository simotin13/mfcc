#ifndef _TYPE_H_
#define _TYPE_H_
#include <stdbool.h>

#define C_TYPES_IDX_VOID    (0)
#define C_TYPES_IDX_CHAR    (1)
#define C_TYPES_IDX_SHORT   (2)
#define C_TYPES_IDX_INT     (3)
#define C_TYPES_IDX_LONG    (4)
#define C_TYPES_IDX_FLOAT   (5)
#define C_TYPES_IDX_DOUBLE  (6)
#define C_TYPES_LEN     (7)

typedef enum {
    ClassLocal,
    ClassStatic,
    ClassExtern,
} StorageClass;

typedef struct {
    int hash;
    int size;
    char name[256];
} Type;
extern const Type c_types[];
Type* type_new(char* name, int size);
extern bool is_same_type(Type* ty1, Type* ty2);

#endif  // _TYPE_H_