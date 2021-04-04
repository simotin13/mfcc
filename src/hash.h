#ifndef _HASH_H_
#define _HASH_H_

extern void hash_init();
extern unsigned int hash(char *str);
extern int hash_set(char *key, void *pVal);
extern void* hash_get(char *key);

#endif	// __HASH_H__