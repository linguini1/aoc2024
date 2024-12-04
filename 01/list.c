#include "list.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
int list_append(list_t *list, const void *element) {

    assert(list->elements != NULL);

    /* We need to add more space. Increase by ~30%. */
    if (list->len + 1 > list->capacity) {
        list->elements = reallocarray(list->elements, list->capacity * 1.3, list->elem_size);
        if (errno != 0) {
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
size_t list_getlen(list_t *list) { return list->len; }

/*
 * Gets a reference to the element at index `i`.
 * @param list The list to index into
 * @param i The index to get from
 * @return A reference to the indexed element if it exists, NULL otherwise.
 */
void *list_getindex(list_t *list, size_t i) {

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
