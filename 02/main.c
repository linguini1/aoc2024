#include "../common/list.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char buffer[BUFSIZ];

#define deref(type, thing) *((type *)((thing)))

bool report_safe(list_t *report, bool with_dampener, size_t skip_index);

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

    size_t total_pure_safe = 0;
    size_t total_damp_safe = 0;

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

        total_pure_safe += report_safe(&report, false, list_len(&report));
        total_damp_safe += report_safe(&report, true, list_len(&report));
    }

    printf("%lu\n", total_pure_safe);
    printf("%lu\n", total_damp_safe);

    return 0;
}

/*
 * Tests if a report is safe.
 * @param report The report to test.
 * @param with_dampener Whether to use the Problem Dampener or not.
 * @param skip_index An index of the element to skip. If greater than the list length, no indexes will be skipped.
 * @return True if safe, false otherwise.
 */
bool report_safe(list_t *report, bool with_dampener, size_t skip_index) {

    /* Any report that is less than two numbers cannot be increasing or decreasing */

    if (list_len(report) < 2) {
        return false;
    }

    /* Check if we are increasing or decreasing on average */

    int last_num = deref(int, list_getindex(report, 0));
    int cur_num;
    unsigned int num_incr = 0;

    for (size_t i = 1; i < list_len(report); i++) {
        cur_num = deref(int, list_getindex(report, i));
        if (cur_num - last_num > 0) {
            num_incr++;
        }
        last_num = cur_num;
    }

    /* If most pairs have an increasing pattern, then we assume increase for this list */

    bool increasing = num_incr > (list_len(report) - num_incr);

    /* If the first index should be skipped, lets offset the start position */

    size_t start_position = 0;
    if (skip_index == 0) {
        start_position++;
    }

    /* Iterate over the list and check each pair. Start one ahead of start position. */

    int diff;
    bool safe_round;
    last_num = deref(int, list_getindex(report, start_position));
    for (size_t i = start_position + 1; i < list_len(report); i++) {

        /* Skip the skip index before any calculations */

        if (i == skip_index) {
            continue;
        }

        safe_round = true; /* Each round starts safe */
        cur_num = deref(int, list_getindex(report, i));
        diff = last_num - cur_num; /* Calculate difference */

        /* Check that we match increasing/decreasing pattern */

        safe_round = safe_round && ((increasing && diff < 0) || (!increasing && diff > 0));

        /* Adjacent levels differ by at least one and at most three */

        safe_round = safe_round && ((abs(diff) >= 1) && (abs(diff) <= 3));

        /* Unsafe round with no dampener triggers a failure. */

        if (!safe_round && !with_dampener) {
            return false;
        }

        /* Safe round gets a pass */

        if (safe_round) {
            last_num = cur_num;
            continue;
        }

        /* If we're here, the dampener was on and we had an unsafe round.
         * The problem can either be resolved by removing this element, or the previous element.
         */

        with_dampener = false; /* Dampener one time use is over */

        /* Skip this element */

        if (report_safe(report, false, i)) {
            return true;
        }

        /* Skip previous element if it is not out of range. If not safe, then this report cannot be saved with the
         * Problem Damper. */

        return (i != 0) && report_safe(report, false, i - 1);
    }

    return true;
}
