#ifndef __VECTOR_H__
#define __VECTOR_H__

typedef struct {
    void **data;
    int capacity;
    int size;
} Vector;

extern Vector* vec_new();
extern void vec_copy(Vector *dst, Vector *src);
extern int vec_push_back(Vector *v, void *data);
extern void *vec_shift(Vector *v);
#endif  // __VECTOR_H__