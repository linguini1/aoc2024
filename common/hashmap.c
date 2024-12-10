#include "hashmap.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Represents the state of a hashmap pair. */
enum pairstate {
    ENT_EMPTY, /* Pair is empty */
    ENT_DEL,   /* Pair used to contain an entry that was deleted */
    ENT_OCC,   /* Pair is occupied */
};

#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) + (uint32_t)(((const uint8_t *)(d))[0]))

/* Fast hashing function taken from http://www.azillionmonkeys.com/qed/hash.html.
 * Author: Paul Hsieh
 * Licensed under LGPL2.1 license.
 * Should be way better than anything I can make.
 */
static uint32_t fasthash(const uint8_t *data, size_t len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof(uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
    case 3:
        hash += get16bits(data);
        hash ^= hash << 16;
        hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
        hash += hash >> 11;
        break;
    case 2:
        hash += get16bits(data);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1:
        hash += (signed char)*data;
        hash ^= hash << 10;
        hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

/* Create a new hashmap
 * @param hmap The hashmap to initialize.
 * @param hasher The hash function to use to hash keys. Leave NULL to use default fast hash by Paul Hsieh.
 * @param init_cap The initial capacity of the backing array
 * @param keysize The size of the keys in bytes
 * @param valsize The size of the values in bytes
 */
void hmap_create(hmap_t *hmap, hash_f hasher, size_t init_cap, size_t keysize, size_t valsize) {
    hmap->len = 0;
    hmap->hasher = hasher;
    if (hasher == NULL) {
        hmap->hasher = fasthash;
    }
    hmap->capacity = init_cap;
    hmap->keysize = keysize;
    hmap->valsize = valsize;
    hmap->pairs = malloc(sizeof(struct hpair) * init_cap);
    memset(hmap->pairs, 0, sizeof(struct hpair) * init_cap); /* Mark all slots empty */
}

/* Get the length of the hashmap (in key, value pairs)
 * @param hmap The hashmap to get the length of.
 * @return The length of the hashmap in key, value pairs
 */
size_t hmap_len(hmap_t const *hmap) { return hmap->len; };

/* Call the hashmap hash function but ensure that computed hashes stay within the capacity of the hash map.
 * @param hmap The hashmap to use
 * @param key The key to hash
 * @return The hash of the key, guaranteed to be within the capacity of the backing array
 */
static size_t hash(const hmap_t *hmap, const void *key) { return hmap->hasher(key, hmap->keysize) % hmap->capacity; }

/* Free a hashmap pair entry.
 * @param pair The pair to free.
 */
static void free_pair(struct hpair *pair) {
    /* No dangling pointer for keys */
    free(pair->key);
    pair->key = NULL;

    /* No dangling pointer for values */
    free(pair->value);
    pair->value = NULL;
}

/* Check the state of a hashmap entry.
 * @param entry The entry to check
 * @return The state of the hashmap entry
 */
static enum pairstate entry_state(struct hpair *pair) {
    if (pair->key == NULL && pair->value == NULL) {
        return ENT_EMPTY;
    }
    if (pair->key == NULL && pair->value == (void *)-1) {
        return ENT_DEL;
    }
    return ENT_OCC;
}

/* Marks a hashmap pair as deleted.
 * @param pair The pair to mark as deleted
 */
static void mark_deleted(struct hpair *pair) {
    pair->key = NULL;
    pair->value = (void *)-1;
}

/* Destroy a hashmap
 * @param hmap The hash map to destroy
 */
void hmap_destroy(hmap_t *hmap) {

    /* Free all pairs */
    for (size_t i = 0; i < hmap->capacity; i++) {
        free_pair(&hmap->pairs[i]);
    }

    /* No dangling pointer for pairs */
    free(hmap->pairs);
    hmap->pairs = NULL;
}

/* Get the value corresponding with `key` if it exists.
 * @param hmap The hashmap to search
 * @param key The key to look for
 * @return A reference to the value associated with `key`, or NULL if the key does not exist.
 */
void *hmap_get(hmap_t const *hmap, void const *key) {
    size_t index = hash(hmap, key);
    size_t i = index;

    do {
        switch (entry_state(&hmap->pairs[i])) {

        /* Nothing ever lived here, so key doesn't exist */
        case ENT_EMPTY:
            return NULL;

        /* Something used to live here, keep probing */
        case ENT_DEL: {
            break;
        }

        /* Something lives here */
        case ENT_OCC: {

            /* Key matches, this is the value we need */
            if (!memcmp(hmap->pairs[i].key, key, hmap->keysize)) {
                return hmap->pairs[i].value;
            }

            /* No match, keep probing */
            break;
        }
        }

        /* Probe whole backing array in a circular loop */

        i = (i + 1) % hmap->capacity;

    } while (i != index);

    /* Loop completed but empty-handed */
    return NULL;
}

/* Remove an entry from the hashmap
 * @param hmap The hashmap to remove from
 * @param key The key to the pair to remove
 */
void hmap_remove(hmap_t *hmap, void const *key) {

    size_t index = hash(hmap, key);
    size_t i = index;

    /* Check the state of the entry, probing until the key is found or we can confirm it never existed */

    do {

        switch (entry_state(&hmap->pairs[i])) {

        /* Entry is empty, nothing ever lived here, nothing to remove any further */
        case ENT_EMPTY:
            return;

        /* If the entry is occupied, check for key equality */
        case ENT_OCC: {

            /* We found a matching entry, remove the pair and mark as deleted */
            if (!memcmp(hmap->pairs[i].key, key, hmap->keysize)) {
                free_pair(&hmap->pairs[i]);
                mark_deleted(&hmap->pairs[i]);
                hmap->len--;
                return;
            }

            /* Keys not equal, continue probing */
            break;
        }

        /* Entry was deleted, continue probing */
        case ENT_DEL: {
            break;
        }
        }

        i = (i + 1) % hmap->capacity; /* Probe whole backing array in circular loop */

    } while (i != index);
}

/* Add or update a pair in the hashmap.
 * @param hmap The hashmap to update
 * @param key The key to use for the pair
 * @param value The value to use for the pair
 */
void hmap_put(hmap_t *hmap, void const *key, void const *value) {
    size_t index = hash(hmap, key);
    size_t i = index;

    // TODO: handle allocating more space if there is not enough

    do {
        /* Check entry state */
        switch (entry_state(&hmap->pairs[i])) {

        /* If the slot is free */
        case ENT_EMPTY:
        case ENT_DEL:

            /* Store key, value pair */
            hmap->pairs[i].key = malloc(hmap->keysize);
            memcpy(hmap->pairs[i].key, key, hmap->keysize);
            hmap->pairs[i].value = malloc(hmap->valsize);
            memcpy(hmap->pairs[i].value, value, hmap->valsize);
            hmap->len++;
            return;

        /* If the slot is occupied */
        case ENT_OCC:

            /* Check if the key matches what's in this slot, and if so replace it. */
            if (!memcmp(hmap->pairs[i].key, key, hmap->keysize)) {
                memcpy(hmap->pairs[i].value, value, hmap->valsize);
                return;
            }

            i = (i + 1) % hmap->capacity; /* Linearly probe until an available slot is found */
            break;
        }
    } while (i != index);
}
