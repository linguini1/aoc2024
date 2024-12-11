#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

static char buffer[BUFSIZ];

bool eq_possible(size_t test, list_t *equation);
bool eq_possible_with_concat(size_t test, list_t *equation);

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
    size_t total_with_concat = 0;
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

        /* Check if equation can be made true with concatenation */
        if (eq_possible_with_concat(test, &equation)) {
            total_with_concat += test;
        }

        /* Destroy equation once done */

        list_destroy(&equation);
    }

    printf("%lu\n", total);
    printf("%lu\n", total_with_concat);

    /* Too low: 31844793361956 */
    /* Close input */

    fclose(puzzle);
}

/*
 * Concatenate two numbers together. Ex: 23 || 4 = 234
 * @param a The first half of the concatenation
 * @param b The second half of the concatenated
 * @return The result of concatenating `a` and `b`
 */
static size_t concatenate(size_t a, size_t b) {

    /* Determine the order of magnitude of `b` */

    size_t mag = 0;
    for (size_t bcp = b; bcp > 0; mag++) {
        bcp /= 10;
    }

    /* Multiply `a` such that it is `mag` orders of magnitude greater than the magnitude of `b`, then add. */

    return (a * pow(10, mag)) + b;
}

/*
 * Evaluate an equation using a bitfield to specify the operators.
 * @param operations
 * @param equation The equation to evaluate
 * @return The result of evaluating the equation
 */
static size_t eq_eval(size_t operations, size_t num_operators, list_t *equation) {

    size_t result = deref(size_t, list_getindex(equation, 0));

    for (size_t i = 1; i < list_getlen(equation); i++) {

        size_t current = deref(size_t, list_getindex(equation, i));
        size_t operator= operations % num_operators;

        switch (operator) {
        case 0:
            result += current;
            break;
        case 1:
            result *= current;
            break;
        case 2:
            result = concatenate(result, current);
            break;
        default:
            printf("Invalid operator %lu on iteration %lu!\n", operator, i);
            exit(EXIT_FAILURE);
            break;
        }

        operations = (operations - operator) / num_operators;
    }
    return result;
}

/*
 * Check if an equation is possible with `num_operators` different operators.
 * NOTE: This assumes operator 0 is add, operator 1 is multiply and operator 2 is concatenation. Only up to 3 operators
 * are supported.
 * @param test The value the equation should equal
 * @param equation The equation values
 * @param num_operators The number of possible operators to use
 */
static bool _eq_possible(size_t test, list_t *equation, size_t num_operators) {

    /*
     * This is done based on the way I performed this initially with binary.
     * For an equation of length `n`, there are `n - 1` operators needed (since all operators are binary in this
     * problem).
     *
     * Therefore, the total possible number of solutions is 2^(`n` - 1) in the binary case, or generally `m`^(`n` - 1)
     * where `m` is the number of operators.
     *
     * The way I would do this with binary is count from 0 up until `m`^(`n` - 1). Each value of the counter is a
     * bitfield where each bit specifies an operator: 0 for add, 1 for multiply. I can then shift the bits right each
     * iteration (iterating up to `n` - 1), mask out everything but the LSB and apply the corresponding operator.
     *
     * I realized that in the general case, shifting right and masking the LSB is like performing a modulo by `n` on the
     * operator bitfield, then subtracting the remainder from the bitfield and dividing it by `n`. This can be expanded
     * to a general case with `m` operators, and now the operation bitfield is no longer a bitfield but simply a number.
     */

    size_t max = pow(num_operators, list_getlen(equation) - 1);
    for (size_t i = 0; i < max; i++) {
        if (test == eq_eval(i, num_operators, equation)) {
            return true;
        }
    }
    return false;
}

/* Check if an equation can be made to equal to the test value with some combination of addition and multiplication
 * operators.
 * @param test The value to test the equation against (what the equation should equal)
 * @param equation The equation to test
 * @return true if the equation could be made to equal the test value, false otherwise
 */
bool eq_possible(size_t test, list_t *equation) { return _eq_possible(test, equation, 2); }

/*
 * Checks if an equation can be made true by using a third concatenation operator.
 * @param test The value the equation should yield once evaluated
 * @param equation The equation to test
 * @return True if the equation can be made to equal the test value, false otherwise
 */
bool eq_possible_with_concat(size_t test, list_t *equation) { return _eq_possible(test, equation, 3); }
