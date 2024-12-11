#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>

/* A dynamic array for any element type. */
typedef struct {
    void *elements;   /* The array of elements */
    size_t len;       /* The length of the list based on stored elements. */
    size_t elem_size; /* Size of the elements in bytes. */
    size_t capacity;  /* Capacity of backing array, in number of elements */
} list_t;

typedef int (*comparison_f)(const void *a, const void *b);
typedef int (*count_f)(const void *e, const void *arg);

void list_create(list_t *list, size_t init_len, size_t elem_size);
void list_destroy(list_t *list);

int list_append(list_t *list, void *element);
size_t list_getlen(list_t const *list);
void *list_getindex(list_t const *list, size_t i);
int list_in(list_t const *list, const void *e);
int list_setindex(list_t *list, size_t i, const void *e);
void list_sort(list_t *list, comparison_f comparison);
size_t list_count(list_t *list, const void *arg, count_f counter);
long long list_index(list_t const *list, const void *e);
void list_pop(list_t *list, void *e);

#endif // _LIST_H_
