#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"
#include "../common/set.h"

#define deref(type, thing) (*((type *)(thing)))
#define GUARD_CHAR '^'
#define FREESPACE '.'

typedef struct {
    int x;
    int y;
} coord_t;

enum direction {
    NORTH,
    SOUTH,
    EAST,
    WEST,
};

typedef struct {
    coord_t pos;
    enum direction dir;
} guard_t;

/* Vector mappings for movement directions */
static const coord_t MOVE[] = {[NORTH] = {0, -1}, [SOUTH] = {0, 1}, [EAST] = {1, 0}, [WEST] = {-1, 0}};

/* Directions after turning right */
static const enum direction RIGHT_TURN[] = {[NORTH] = EAST, [EAST] = SOUTH, [SOUTH] = WEST, [WEST] = NORTH};

/* String representation of direction for debugging */
static const char *DIRSTRING[] = {[NORTH] = "NORTH", [SOUTH] = "SOUTH", [EAST] = "EAST", [WEST] = "WEST"};

/* Check if coordinate is out of bounds */
static int out_of_bounds(coord_t coords, size_t xlen, size_t ylen) {
    return (coords.x < 0 || coords.y < 0 || coords.x >= xlen || coords.y >= ylen);
}

/* Add two coordinates */
static coord_t coord_add(coord_t a, coord_t b) { return (coord_t){.x = a.x + b.x, .y = a.y + b.y}; }

void record_visited(guard_t guard, list_t *grid, size_t xlen, size_t ylen, set_t *visited);

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

    /* Parse the input into a grid */

    list_t grid;
    list_create(&grid, 1000, sizeof(char));

    size_t ylen = 0;
    size_t xlen = 0;
    char c;

    while (!feof(puzzle)) {

        c = fgetc(puzzle);

        /* End at EOF */
        if (c == EOF) {
            break;
        }

        /* Skip newlines */
        if (c == '\n') {
            ylen++;
            continue;
        }

        list_append(&grid, &c);
    }
    xlen = list_len(&grid) / ylen;

    /* Get starting position of guard */

    char guard_char = GUARD_CHAR;
    size_t start = list_index(&grid, &guard_char);

    /* Replace the guard with free space since we can walk over ourselves */

    char freespace = FREESPACE;
    list_setindex(&grid, start, &freespace);

    guard_t guard = {
        .pos = {.x = start % ylen, .y = (start - (start % ylen)) / ylen},
        .dir = NORTH,
    };

    /* Create a set of visited locations */

    set_t visited;
    set_create(&visited, NULL, BUFSIZ, sizeof(coord_t));
    record_visited(guard, &grid, xlen, ylen, &visited);

    printf("%lu\n", set_len(&visited));

    /* Close input */

    list_destroy(&grid);
    set_destroy(&visited);
    fclose(puzzle);
}

/* Records all the locations visited by the guard during its journey.
 * @param guard The guard with its initial starting position and direction
 * @param grid The map
 * @param xlen The number of columns in the map
 * @param ylen The number of rows in the map
 * @param visited A set in which to store the visited locations
 */
void record_visited(guard_t guard, list_t *grid, size_t xlen, size_t ylen, set_t *visited) {

    set_add(visited, &guard.pos); /* Record start position */

    /* Start state machine logic */

    coord_t new_pos;

    for (;;) {
        new_pos = coord_add(guard.pos, MOVE[guard.dir]);

        /* If the guard went out of bounds, we're done */

        if (out_of_bounds(new_pos, xlen, ylen)) {
            break;
        }

        /* If the guard would hit an object, turn 90 degrees right and continue forward */

        if (deref(char, list_getindex(grid, new_pos.y * ylen + new_pos.x)) == '#') {
            guard.dir = RIGHT_TURN[guard.dir];
            continue;
        }

        /* If we're in the grid and didn't hit anything, we're here. Record the position */

        guard.pos = new_pos;
        set_add(visited, &guard.pos); /* Record start position */
    }
}
