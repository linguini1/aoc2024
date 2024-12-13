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
    int64_t x;
    int64_t y;
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
    printf("Machine((%ld, %ld), (%ld, %ld), (%ld, %ld))", machine->a.x, machine->a.y, machine->b.x, machine->b.y,
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
        machine_t *machine = list_getindex(&machines, i);
        coord_t combo = best_combo(machine);
        total += cost(combo);
    }

    printf("%zu\n", total);

    /* Close input */

    fclose(puzzle);
}

/* Determines the best combination of a & b buttons to win the prize for the lowest price.
 * @param machine The machine to beat
 * @return A coordinate pair where x represents the number of a presses and y is b presses. If both numbers are 0, this
 * prize machine can't be beat.
 */
coord_t best_combo(const machine_t *machine) {

    coord_t combo;
    for (combo.x = 0; combo.x < 100; combo.x++) {
        for (combo.y = 0; combo.y < 100; combo.y++) {
            if ((combo.x * machine->a.x + combo.y * machine->b.x == machine->prize.x) &&
                (combo.x * machine->a.y + combo.y * machine->b.y == machine->prize.y)) {
                return combo;
            }
        }
    }
    return (coord_t){.x = 0, .y = 0};
}
