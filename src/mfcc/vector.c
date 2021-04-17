#include <stdlib.h>

#include "mfcc.h"
#include "debug.h"
#include "vector.h"

#define VEC_DEF_CAPACITY_SIZE   (16)

Vector *vec_new()
{
    Vector *v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void *) * VEC_DEF_CAPACITY_SIZE);
    v->capacity = VEC_DEF_CAPACITY_SIZE;
    v->size = 0;
    return v;
}

void vec_copy(Vector *dst, Vector *src)
{
    int i;
    dst->capacity = src->capacity;
    dst->size = src->size;
    for (i = 0; i < dst->capacity; i++) {
        dst->data[i] = malloc(sizeof(void *));
        dst->data[i] = src->data[i];
    }
    return;
}

int vec_push(Vector *v, void *data)
{
    if (v->capacity == v->size) {
        v->data = realloc(v->data, sizeof(void *) * v->capacity * 2);
        v->capacity = v->capacity * 2;
    }
    v->data[v->size] = data;
    v->size++;
    return 0;
}
