#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BACKET_SIZE         (4096)

typedef struct _HASH_ENTRY {
    char key[256];
    void *val;
} HASH_ENTRY;

static HASH_ENTRY hash_backet[BACKET_SIZE];

#define FNV_PRIME_32        (1677619)
#define FNV_OFFSET_32       (2166136261)
static unsigned int hash(char *str)
{
    // use FNV Hash Algorithm
    int i;
    unsigned int h = FNV_PRIME_32;
    char* p = str;
    while(*p) {
        h ^= *p;
        h *= FNV_OFFSET_32;
        p++;
    }
    return h;
}

void hash_init()
{
    memset(hash_backet, 0, sizeof(HASH_ENTRY) * BACKET_SIZE);
}

int hash_set(char *key, void *pVal) 
{
    unsigned int h = hash(key);
    unsigned int idx = h % BACKET_SIZE;
    if (hash_backet[idx].val != NULL) {
        fprintf(stderr, "hash collison!\n");
        assert(0);
        return -1;
    }

    strcpy(hash_backet[idx].key, key);
    hash_backet[idx].val = pVal;
    return 0;
}

void* hash_get(char *key)
{
    unsigned int h = hash(key);
    unsigned int idx = h % BACKET_SIZE;
    return hash_backet[idx].val;
}
