#include "list.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buffer[BUFSIZ];

int ascending_order(const void *a, const void *b) { return *(int *)(a) > *(int *)(b); }

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Provide the name of the file to use as puzzle input.");
        return EXIT_FAILURE;
    }

    /* Create two lists of integers */

    list_t ls;
    list_create(&ls, 50, sizeof(int));
    list_t rs;
    list_create(&rs, 50, sizeof(int));

    /* Open the puzzle input */

    FILE *puzzle = fopen(argv[1], "r");
    if (puzzle == NULL) {
        fprintf(stderr, "Failed to open puzzle input file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parse each line into two integers */

    while (!feof(puzzle)) {

        /* Get next line */
        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            if (feof(puzzle)) {
                break;
            } else {
                fprintf(stderr, "Error while reading file: %s\n", strerror(ferror(puzzle)));
                exit(EXIT_FAILURE);
            }
        }

        /* Parse into two integers and store them */

        int left = atoi(strtok(buffer, " "));
        int right = atoi(strtok(NULL, " "));

        list_append(&ls, &left);
        list_append(&rs, &right);
    }

    /* Sort the lists */

    list_sort(&ls, ascending_order);
    list_sort(&rs, ascending_order);

    /* Compute the sum of the distances between each element */

    size_t sum = 0;
    for (size_t i = 0; i < list_getlen(&ls); i++) {
        sum += abs(*((int *)(list_getindex(&ls, i))) - *((int *)(list_getindex(&rs, i))));
    }

    /* Print out the sum */
    printf("%lu\n", sum);

    return EXIT_SUCCESS;
}
