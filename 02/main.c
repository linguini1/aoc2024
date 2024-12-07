#include "../common/list.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buffer[BUFSIZ];

#define deref(type, thing) *((type *)((thing)))

bool report_safe(list_t *report);

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Provide the name of the file to use as puzzle input.");
        return EXIT_FAILURE;
    }

    /* Open the puzzle input */

    FILE *puzzle = fopen(argv[1], "r");
    if (puzzle == NULL) {
        fprintf(stderr, "Failed to open puzzle input file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parse each line (report) */

    size_t total_safe = 0;
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

        /* Process the line into a list (report) */

        list_t report;
        list_create(&report, 20, sizeof(int));

        char *number_str = strtok(buffer, " ");
        int number;

        do {
            number = atoi(number_str);
            list_append(&report, &number);
        } while ((number_str = strtok(NULL, " ")) != NULL);

        /* Ensure the list meets requirements */

        total_safe += report_safe(&report);
    }

    printf("%lu\n", total_safe);

    return 0;
}

/*
 * Tests if a report is safe.
 * @param report The report to test.
 * @return True if safe, false otherwise.
 */
bool report_safe(list_t *report) {

    /* Any report that is less than two numbers cannot be increasing or decreasing */

    if (list_getlen(report) < 2) {
        return false;
    }

    /* Start by grabbing the first two elements to see if the list is increasing or decreasing */

    int last_num = deref(int, list_getindex(report, 0));
    int cur_num = deref(int, list_getindex(report, 1));

    bool increasing = false;

    if (last_num - cur_num > 0) {
        increasing = true;
    }

    /* Make sure that all numbers meet the criteria. Loop starts at 1 to skip over first index. */

    int diff;
    for (size_t i = 1; i < list_getlen(report); i++) {

        cur_num = deref(int, list_getindex(report, i));
        diff = last_num - cur_num; /* Calculate difference */

        /* Check that we match increasing/decreasing pattern */

        if ((increasing && diff < 0) || (!increasing && diff > 0)) {
            return false;
        }

        /* Adjacent levels differ by at least one and at most three */

        if (abs(diff) > 3 || abs(diff) < 1) {
            return false;
        }

        /* Record the last number */

        last_num = cur_num;
    }

    return true;
}
