#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

static char buffer[BUFSIZ];

bool eq_possible(size_t test, list_t *equation);

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

    /* Parse input into calibration equations */

    size_t total = 0;
    for (;;) {

        /* Get next input line */
        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        /* Parse out test number */

        size_t test = strtoul(strtok(buffer, ":"), NULL, 10);

        /* Add equation values to list */

        list_t equation;
        list_create(&equation, 15, sizeof(size_t));

        char *tok;
        size_t cur;
        while ((tok = strtok(NULL, " ")) != NULL) {
            cur = strtoul(tok, NULL, 10);
            list_append(&equation, &cur);
        }

        /* Check if equation can be made true */

        if (eq_possible(test, &equation)) {
            total += test;
        }

        /* Destroy equation once done */

        list_destroy(&equation);
    }

    printf("%lu\n", total);

    /* Close input */

    fclose(puzzle);
}

/*
 * Evaluate an equation using a bitfield to specify the operators.
 * @param operations The operator bitfield where 0 is addition and 1 is multiplication.
 * @param equation The equation to evaluate
 * @return The result of evaluating the equation
 */
static size_t eq_eval(size_t operations, list_t *equation) {

    size_t result = deref(size_t, list_getindex(equation, 0));

    for (size_t i = 1; i < list_getlen(equation); i++, operations >>= 1) {
        if (operations & 0x1) {
            result *= deref(size_t, list_getindex(equation, i));
        } else {
            result += deref(size_t, list_getindex(equation, i));
        }
    }
    return result;
}

/* Check if an equation can be made to equal to the test value with some combination of addition and multiplication
 * operators.
 * @param test The value to test the equation against (what the equation should equal)
 * @param equation The equation to test
 * @return true if the equation could be made to equal the test value, false otherwise
 */
bool eq_possible(size_t test, list_t *equation) {

    /* Generate all possible permutations of addition and multiplications operations by using binary.
     * Addition: 0
     * Multiplication: 1
     *
     * For a list of length `n`, we count from 0 until 2^(`n` - 1).
     * For a list of length 3, we count from 0 until 4 (2^2).
     * Doing so generates 00, 01, 10, 11. These are all possible operators combinations for that equation.
     */

    size_t max = 1 << (list_getlen(equation) - 1);
    for (size_t i = 0; i < max; i++) {
        if (test == eq_eval(i, equation)) {
            return true;
        }
    }

    return false;
}
