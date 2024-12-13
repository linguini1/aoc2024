#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

/*
 * Constructs a new list.
 * @param list A pointer to the list to initialize
 * @param init_len the starting length of the list
 * @param elem_size the size of the elements to be stored in the list
 */
void list_create(list_t *list, size_t init_len, size_t elem_size) {
    list->capacity = init_len;
    list->elem_size = elem_size;
    list->len = 0;
    list->elements = malloc(init_len * elem_size);
    if (list->elements == NULL) {
        list->len = 0;
        return;
    }
}

/*
 * Frees the memory in the list.
 * @param list The list to free.
 */
void list_destroy(list_t *list) {
    free(list->elements);
    list->elements = NULL;
}

/*
 * Copy an item to the end of the list.
 * @param `list` A pointer to the list to append to.
 * @param `elem` the element to append.
 * @return 0 on success, errno on failure.
 */
int list_append(list_t *list, void *element) {

    assert(list->elements != NULL);

    /* We need to add more space. Increase by ~30%. */
    if (list->len + 1 > list->capacity) {
        list->elements = realloc(list->elements, list->capacity * list->elem_size * 1.3);
        if (list->elements == NULL) {
            return errno;
        }

        // Reallocated successfully, update capacity
        list->capacity *= 1.3;
    }

    /* Add the element */

    memcpy(((uint8_t *)(list->elements) + (list->len * list->elem_size)), element, list->elem_size);
    list->len++;

    return 0;
}

/* Returns the length of the list
 * @param list The list to get the length of
 * @return The length of the list
 */
size_t list_len(const list_t *list) { return list->len; }

/*
 * Gets a reference to the element at index `i`.
 * @param list The list to index into
 * @param i The index to get from
 * @return A reference to the indexed element if it exists, NULL otherwise.
 */
void *list_getindex(list_t const *list, size_t i) {

    if (i > list->len) {
        return NULL;
    }
    assert(list->elements != NULL);

    return (uint8_t *)(list->elements) + (i * list->elem_size);
}

/*
 * Sets the element at index `i`.
 * @param list The list to index into
 * @param i The index to set
 * @param e The value to set the element at index `i`
 * @return 0 on success, EINVAL if the index does not exist
 */
int list_setindex(list_t *list, size_t i, const void *e) {

    if (i > list->len) {
        return EINVAL;
    }
    assert(list->elements != NULL);

    memcpy((uint8_t *)(list->elements) + (i * list->elem_size), e, list->elem_size);
    return 0;
}

/*
 * Sorts the list.
 * @param list The list to sort
 * @param comparison The function to use to compare two items
 */
void list_sort(list_t *list, comparison_f comparison) { qsort(list->elements, list->len, list->elem_size, comparison); }

/*
 * Counts the number of times a condition is met in the list.
 * @param list The list to perform the count on
 * @param arg The argument to pass the counting function
 * @param counter The counting function which returns 1 to increment the counter and 0 to do nothing
 * @return The resulting count.
 */
size_t list_count(list_t *list, const void *arg, count_f counter) {
    size_t count = 0;
    for (size_t i = 0; i < list->len; i++) {
        count += counter((list->elements + (list->elem_size * i)), arg);
    }
    return count;
}

/*
 * Checks if the given element is in the list at least once
 * @param list The list to check
 * @param e The element to look for
 * @return 0 if element is not present, 1 if present
 */
int list_in(list_t const *list, const void *e) {
    for (size_t i = 0; i < list->len; i++) {
        if (!memcmp(list_getindex(list, i), e, list->elem_size)) {
            return 1;
        }
    }
    return 0;
}

/*
 * Returns the index of the first occurrence of `e`.
 * @param list The list to search
 * @param e The element to look for
 * @return A positive index corresponding to the first occurrence of the element, or -1 if not found.
 */
long long list_index(list_t const *list, const void *e) {
    for (size_t i = 0; i < list->len; i++) {
        if (!memcmp(list_getindex(list, i), e, list->elem_size)) {
            return i;
        }
    }
    return -1;
}

/* Pop an element from the end of the list and store it in `e`.
 * @param list The list to pop from
 * @param e Where to store the popped element. Pass `NULL` if element can be discarded.
 */
void list_pop(list_t *list, void *e) {
    if (e != NULL) {
        memcpy(e, (uint8_t *)(list->elements) + ((list->len - 1) * list->elem_size), list->elem_size);
    }
    list->len--;
}
