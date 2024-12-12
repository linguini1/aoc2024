#include <stdint.h>
#include <string.h>

#include "set.h"

enum slot_e {
    SLOT_EMPTY = 0, /* Empty slot */
    SLOT_DEL = 1,   /* Something was here, but got deleted */
    SLOT_OCC = 2,   /* Slot occupied */
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

/* Create a new set
 * @param set The set to initialize
 * @param hasher The hash function to use. Pass NULL to use the default hasher by Paul Hsieh
 * @param init_cap The initial capacity of the set
 * @param elemsize The size of each element in bytes
 */
void set_create(set_t *set, hash_f hasher, size_t init_cap, size_t elemsize) {
    set->elemsize = elemsize;
    set->hasher = hasher;
    if (hasher == NULL) set->hasher = fasthash;
    set->capacity = init_cap;
    set->len = 0;

    /* Allocate a little extra space for tracking slot state */
    size_t byte_cap = init_cap * elemsize + init_cap * sizeof(uint8_t);
    set->elems = malloc(byte_cap);
    memset(set->elems, 0, byte_cap);
}

/* Free a set.
 * @param set The set to destroy.
 */
void set_destroy(set_t *set) {
    free(set->elems);
    memset(set, 0, sizeof(set_t));
}

/* Get the length of the set.
 * @param set The set to get the length of
 * @return The length of the set (how many elements it currently contains)
 */
size_t set_len(set_t const *set) { return set->len; }

/* Hash an element into an index into the backing array.
 * @param set The set to hash the element for.
 * @param elem The element to hash
 * @return The hashed element, guaranteed to be a number that can safely index the set's backing array.
 */
static size_t hash(const set_t *set, const void *elem) { return set->hasher(elem, set->elemsize) % set->capacity; }

/*
 * Gets the slot at index `i` and its state.
 * @param set The set to get the slot from.
 * @param i The slot's index
 * @param state A pointer to the slot's state
 * @return A pointer to the start of the element.
 */
static void *get_slot(const set_t *set, size_t i, uint8_t **state) {

    /* Get the current slot, which has the state as the header */
    *state = (uint8_t *)(set->elems) + ((set->elemsize + 1) * i);
    return *state + 1; /* Element is 1 byte after state */
}

/* Add an element to the set
 * @param set The set to add to.
 * @param elem The element to add.
 */
void set_add(set_t *set, const void *elem) {
    size_t index = hash(set, elem);
    size_t i = index;
    void *cur;
    uint8_t *state;

    /* Linearly probe for an empty slot */

    do {

        cur = get_slot(set, i, &state);

        /* Copy element into first available slot */

        if (*state == SLOT_EMPTY || *state == SLOT_DEL) {
            memcpy(cur, elem, set->elemsize);
            *state = SLOT_OCC;
            set->len++;
            break;
        }

        /* If slot is occupied, check if the element is already there. Don't add duplicates */

        if (*state == SLOT_OCC && !memcmp(cur, elem, set->elemsize)) {
            return;
        }

        i = (i + 1) % set->capacity; /* Loop around */

    } while (i != index);
}

/* Remove an element to the set
 * @param set The set to remove from
 * @param elem The element to remove
 */
void set_remove(set_t *set, const void *elem) {
    size_t index = hash(set, elem);
    size_t i = index;
    void *cur;
    uint8_t *state;

    /* Linearly probe for matching slot */

    do {

        /* Get the current slot */

        cur = get_slot(set, i, &state);

        /* If an empty slot was found, end here because it means nothing was ever here */

        if (*state == SLOT_DEL) return;

        /* A matching element was found, delete it */

        if (*state == SLOT_OCC && !memcmp(cur, elem, set->elemsize)) {
            *state = SLOT_DEL;
            set->len--;
            break;
        }

        /* If matching slot not found, continue looping */

        i = (i + 1) % set->capacity; /* Loop around */

    } while (i != index);
}

/* Check if a set contains an element.
 * @param set The set to check
 * @param elem The element to look for
 * @return 0 if the element is not in the set, 1 otherwise
 */
int set_contains(set_t const *set, const void *elem) {
    size_t index = hash(set, elem);
    size_t i = index;
    void *cur;
    uint8_t *state;

    /* Linearly probe for matching slot */

    do {

        /* Get the current slot */

        cur = get_slot(set, i, &state);

        /* If an empty slot was found, end here because it means nothing was ever here */

        if (*state == SLOT_DEL) return 0;

        /* A matching element was found, success! */

        if (*state == SLOT_OCC && !memcmp(cur, elem, set->elemsize)) {
            return 1;
        }

        /* If matching slot not found, continue looping */

        i = (i + 1) % set->capacity; /* Loop around */

    } while (i != index);

    /* We looped until we wrapped back around and found nothing */
    return 0;
}

/* Iterate over elements in the set.
 * @param set The set to iterate over
 * @param i Contains state between calls. Pass with initial value of 0.
 * @param elem A pointer to where to store the reference to the current element
 * @return NULL when all values have been iterated over, same pointer as in `elem` otherwise.
 */
void *set_iter(set_t const *set, size_t *i, void **elem) {
    uint8_t *state;
    void *cur;

    for (; *i < set->capacity; (*i)++) {

        /* If something is in this slot, return it */

        cur = get_slot(set, *i, &state);
        if (*state == SLOT_OCC) {
            *elem = cur;
            (*i)++;
            return elem;
        }
    }

    return NULL;
}
