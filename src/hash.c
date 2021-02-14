#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BACKET_SIZE         (1021)

typedef struct _HASH_ENTRY {
    char key[128];
    void *val;
} HASH_ENTRY;

static HASH_ENTRY hash_backet[BACKET_SIZE];

#if 0
static void print_bin(unsigned int val)
{
    int i;
    unsigned int mask = 0x80000000;
    for (i = 0; i < 32; i++) {
        if (val & mask) {
            fprintf(stdout, "1");
        } else {
            fprintf(stdout, "0");
        }

        mask >>= 1;
    }
    fprintf(stdout, "\n");
}

static unsigned int l_rotate(unsigned int val, unsigned int count)
{
    int i = 0;
    unsigned char cf = 0;
    for (i = 0; i < count; i++) {
        if (val & 0x80000000) {
            cf = 1;
        }
        val <<= 1;
        if (cf) {
            val |= 1;
        }
    }

    return val;
}
#endif

static unsigned int hash(char *str)
{
    int i;
    unsigned int h = 0;
    int len = strlen(str);
    for (i = 0; i < len; i++) {
        h += str[i];
        h = h * 0x3F1B2351;
    }

    fprintf(stdout, "h:%08X\n", h);
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
    fprintf(stdout, "idx:%d\n", idx);

    if (strcmp(hash_backet[idx].key, key) != 0) {
        if (hash_backet[idx].val != NULL) {
            fprintf(stderr, "hash collison!\n");
            assert(0);
            return -1;
        }

        strcpy(hash_backet[idx].key, key);
    }

    hash_backet[idx].val = pVal;
    return 0;
}

void* hash_get(char *key)
{
    unsigned int h = hash(key);
    unsigned int idx = h % BACKET_SIZE;
    return hash_backet[idx].val;
}
