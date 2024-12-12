#ifndef _SET_H_
#define _SET_H_

#include <stdint.h>
#include <stdlib.h>

/* Generic hash function */
typedef uint32_t (*hash_f)(const uint8_t *data, size_t len);

/* Represents a set */
typedef struct {
    size_t elemsize; /* Size of each element */
    size_t capacity; /* Capacity of backing array */
    size_t len;      /* Length of the set */
    hash_f hasher;   /* The hash function to hash elements */
    void *elems;     /* The elements in the set */
} set_t;

void set_create(set_t *set, hash_f hasher, size_t init_cap, size_t elemsize);
void set_destroy(set_t *set);
size_t set_len(set_t const *set);
void set_add(set_t *set, const void *elem);
void set_remove(set_t *set, const void *elem);
int set_contains(set_t const *set, const void *elem);
void *set_iter(set_t const *set, size_t *i, void **elem);

#endif // _SET_H_
