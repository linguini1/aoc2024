#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

typedef struct {
    size_t x;
    size_t y;
} coord_t;

typedef struct {
    coord_t a;
    coord_t b;
    coord_t prize;
} machine_t;

static char buffer[BUFSIZ];

/* Add two coordinates */

static coord_t coord_add(coord_t a, coord_t b) { return (coord_t){.x = a.x + b.x, .y = a.y + b.y}; }

static void print_machine(machine_t const *machine, int newline) {
    printf("Machine((%zu, %zu), (%zu, %zu), (%zu, %zu))", machine->a.x, machine->a.y, machine->b.x, machine->b.y,
           machine->prize.x, machine->prize.y);
    if (newline) {
        printf("\n");
    }
}

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

    /* Close input */

    fclose(puzzle);
}
