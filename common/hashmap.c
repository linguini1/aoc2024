#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

/* Represents the state of a hashmap pair. */
enum pairstate {
    ENT_EMPTY, /* Pair is empty */
    ENT_DEL,   /* Pair used to contain an entry that was deleted */
    ENT_OCC,   /* Pair is occupied */
};

/* Create a new hashmap
 * @param hmap The hashmap to initialize.
 * @param hasher The hash function to use to hash keys
 * @param init_cap The initial capacity of the backing array
 * @param keysize The size of the keys in bytes
 * @param valsize The size of the values in bytes
 */
void hmap_create(hmap_t *hmap, hash_f hasher, size_t init_cap, size_t keysize, size_t valsize) {
    hmap->len = 0;
    hmap->hasher = hasher;
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
static size_t hash(const hmap_t *hmap, const void *key) { return hmap->hasher(key) % hmap->capacity; }

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

        /* Nothing ever live here, so key doesn't exist */
        case ENT_EMPTY:
            return NULL;

        /* Something used to live here, keep probing */
        case ENT_DEL: {
            break;
        }

        /* Something lives here */
        case ENT_OCC: {
            /* Key matches, this is the value we need */
            if (!strncmp(hmap->pairs[index].key, key, hmap->keysize)) {
                return hmap->pairs[index].value;
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
            if (!strncmp(hmap->pairs[i].key, key, hmap->keysize)) {
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
            i = (i + 1) % hmap->capacity; /* Linearly probe until an available slot is found */
            break;
        }
    } while (i != index);
}
