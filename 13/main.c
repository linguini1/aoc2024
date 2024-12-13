#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

#define A_COST 3
#define B_COST 1

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct {
    coord_t a;
    coord_t b;
    coord_t prize;
} machine_t;

static char buffer[BUFSIZ];

/* Add two coordinates */

static coord_t coord_add(coord_t a, coord_t b) { return (coord_t){.x = a.x + b.x, .y = a.y + b.y}; }

/* Print out a claw machine */

static void print_machine(machine_t const *machine, int newline) {
    printf("Machine((%d, %d), (%d, %d), (%d, %d))", machine->a.x, machine->a.y, machine->b.x, machine->b.y,
           machine->prize.x, machine->prize.y);
    if (newline) {
        printf("\n");
    }
}

/* Calculate the cost of a series of presses */

static size_t cost(coord_t presses) { return presses.x * A_COST + presses.y * B_COST; }

coord_t best_combo(const machine_t *machine);

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Provide the name of the file to use as puzzle input.\n");
        return EXIT_FAILURE;
    }

    /* Open the puzzle input */

    FILE *puzzle = fopen(argv[1], "r");
    if (puzzle == NULL) {
        fprintf(stderr, "Failed to open puzzle input file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parse puzzle input into claw machines */

    list_t machines;
    list_create(&machines, 50, sizeof(machine_t));

    char *tok;
    machine_t cur;
    for (;;) {

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        /* Get the button A information */

        tok = strtok(buffer, "+");
        tok = strtok(NULL, ",");
        cur.a.x = strtoul(tok, NULL, 10);
        tok = strtok(NULL, "+");
        tok = strtok(NULL, ",");
        cur.a.y = strtoul(tok, NULL, 10);

        /* Get the button B information */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        tok = strtok(buffer, "+");
        tok = strtok(NULL, ",");
        cur.b.x = strtoul(tok, NULL, 10);
        tok = strtok(NULL, "+");
        tok = strtok(NULL, ",");
        cur.b.y = strtoul(tok, NULL, 10);

        /* Get the prize information */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        tok = strtok(buffer, "=");
        tok = strtok(NULL, ",");
        cur.prize.x = strtoul(tok, NULL, 10);
        tok = strtok(NULL, "=");
        tok = strtok(NULL, ",");
        cur.prize.y = strtoul(tok, NULL, 10);

        /* Record the machine */

        list_append(&machines, &cur);

        /* Skip the coming empty line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;
    }

    /* Calculate the total cost */

    size_t total = 0;
    for (size_t i = 0; i < list_len(&machines); i++) {
        coord_t combo = best_combo(list_getindex(&machines, i));
        printf("A: %d, B: %d\n", combo.x, combo.y);
        total += cost(combo);
    }

    printf("%zu\n", total);

    /* Close input */

    fclose(puzzle);
}

/* First machine: 80A 40B
 * 8400 % 22 = 18       ;
 * 5400 % 34 = 28       ;
 *
 * 8400 % 94 = 34       ;
 * 5400 % 67 = 40       ;
 *
 * Second machine, no solution.
 * 12748 % 26 = 8         ;
 * 12176 % 66 = 32        ;
 *
 * 12748 % 67 = 18        ;
 * 12176 % 21 = 17        ;
 *
 * Third machine: 38A & 86B
 * 7879 % 17 = 8        ;
 * 6450 % 86 = 0        ;
 *
 * 7879 % 84 = 67       ;
 * 6450 % 37 = 12       ;
 *
 * (A * a.x) + (B * b.x) = prize.x
 * (A * a.y) + (B * b.y) = prize.y
 *
 * 2 equations, two unknowns
 *
 * A * a.x = prize.x - (B * b.y)
 * A = (prize.x - (B * b.y)) / a.x
 *
 * A * a.y = prize.y - (B * b.y)
 * A = (prize.y - (B * b.y)) / a.y
 * (prize.x - (B * b.y)) / a.x = (prize.y - (B * b.y)) / a.y
 * prize.x - (B * b.y)  = (a.x * (prize.y - (B * b.y))) / a.y
 *
 */

/* Determines the best combination of a & b buttons to win the prize for the lowest price.
 * @param machine The machine to beat
 * @return A coordinate pair where x represents the number of a presses and y is b presses. If both numbers are 0, this
 * prize machine can't be beat.
 */
coord_t best_combo(const machine_t *machine) {

    coord_t presses = {.x = 0, .y = 0};
    coord_t position = {.x = 0, .y = 0};

    presses.y = (machine->prize.y * machine->a.x - machine->prize.x * machine->a.y) /
                (-machine->b.x * machine->a.y + machine->b.y * machine->a.x);
    presses.x = (machine->prize.x - presses.y * machine->b.x) / machine->a.x;

    /* No solution exists for this claw machine since we estimate each button shouldn't be pressed more than 100 times
     * to win a prize.
     */
    if (presses.x > 100 || presses.y > 100) {
        presses.x = 0;
        presses.y = 0;
    }

    /* TODO 28314 too high */
    return presses;
}
