#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/hashmap.h"
#include "../common/list.h"

#define deref(type, thing) *((type *)(thing))

static char buffer[BUFSIZ];

typedef struct {
    int before;
    int after;
} rule_t;

bool ordered_correctly(list_t *update, hmap_t *rules);

/* Key for hashmap */
static size_t hash(const void *key) { return *(int *)key; }

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Provide the name of the file to use as puzzle input.\n");
        return EXIT_FAILURE;
    }

    /* Open the puzzle input */

    FILE *puzzle = fopen(argv[1], "r");
    if (puzzle == NULL) {
        fprintf(stderr, "Failed to open puzzle input file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Create hashmap of rules */

    hmap_t rulebook;
    hmap_create(&rulebook, hash, BUFSIZ, sizeof(int), sizeof(list_t));

    int before;
    int after;
    for (;;) {

        fgets(buffer, sizeof(buffer), puzzle);
        if (strlen(buffer) <= 1) break; /* Now the updates start */

        /* Parse into rule entry */

        before = atoi(strtok(buffer, "|"));
        after = atoi(strtok(NULL, "|"));

        /* If the list of rules for this number exists, add to it */

        list_t *rules = hmap_get(&rulebook, &before);
        if (rules != NULL) {
            list_append(rules, &after);
            continue;
        }

        /* If it does not exist, then create a list. */

        list_t newrules;
        list_create(&newrules, 20, sizeof(int));
        list_append(&newrules, &after);

        /* List is copied on put, so it's okay that it was allocated on the stack */
        hmap_put(&rulebook, &before, &newrules);
    }

    /* Iterate over updates */

    list_t update;
    char *tok;
    int page;

    size_t total = 0;
    for (;;) {

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        /* Parse into an update (list of page numbers) */

        list_create(&update, 50, sizeof(int));
        tok = strtok(buffer, ",");
        do {
            page = atoi(tok);
            list_append(&update, &page);
            tok = strtok(NULL, ",");
        } while (tok != NULL);

        /* Process update list to verify correct order TODO */

        if (ordered_correctly(&update, &rulebook)) {
            total += deref(int, list_getindex(&update, list_getlen(&update) / 2));
        }

        /* Delete update list to be made fresh */

        list_destroy(&update);
    }

    printf("%lu\n", total);

    /* Close input */

    // TODO free created lists
    hmap_destroy(&rulebook);
    fclose(puzzle);
}

bool ordered_correctly(list_t *update, hmap_t *rules) {

    /* Check each list element precedes what it should */
    for (size_t i = 0; i < list_getlen(update); i++) {

        int elem = deref(int, list_getindex(update, i));
        list_t *precedes = hmap_get(rules, &elem);

        /* No rule exists for this element, therefore its order doesn't matter */

        if (precedes == NULL) {
            continue;
        }

        /* Make sure everything after this element is something it can precede */

        for (size_t j = i + 1; j < list_getlen(update); j++) {
            int after = deref(int, list_getindex(update, j));
            if (!list_in(precedes, &after)) {
                return false; /* Number after element is not in its precedes list */
            }
        }

        /* Make sure nothing before this element is something it must precede */
        for (size_t j = i; j > 0; j--) {
            int before = deref(int, list_getindex(update, j));
            if (list_in(precedes, &before)) {
                return false;
            }
        }
    }

    return true;
}
