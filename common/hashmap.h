#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdlib.h>

/* Generic hash function */
typedef size_t (*hash_f)(const void *);

/* Hashmap pair */
struct hpair {
    void *key;   /* The pair's key */
    void *value; /* The pair's value */
};

/* A hash map. */
typedef struct {
    size_t capacity;     /* Capacity of backing array in number of pairs */
    size_t keysize;      /* Size of keys */
    size_t valsize;      /* Size of values */
    size_t len;          /* Number of key, value pairs stored */
    struct hpair *pairs; /* Backing array of pairs */
    hash_f hasher;       /* Hash function to use */
} hmap_t;

void hmap_create(hmap_t *hmap, hash_f hasher, size_t init_cap, size_t keysize, size_t valsize);
void hmap_destroy(hmap_t *hmap);
void *hmap_get(hmap_t const *hmap, void const *key);
void hmap_remove(hmap_t *hmap, void const *key);
void hmap_put(hmap_t *hmap, void const *key, void const *value);
size_t hmap_len(hmap_t const *hmap);

#endif // _HASHMAP_H_
