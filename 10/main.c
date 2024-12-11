#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"
#include "../common/set.h"

#define deref(type, thing) (*((type *)(thing)))

#define TRAILHEAD 0
#define TRAILEND 9

static char buffer[BUFSIZ];

typedef struct {
    int x;
    int y;
} coord_t;

/* Neighbouring cells represented as vectors */
static const coord_t NEIGHBOURS[] = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};

/* Add two coordinates */
static coord_t coord_add(coord_t a, coord_t b) { return (coord_t){.x = a.x + b.x, .y = a.y + b.y}; }

/* Check if coordinate is out of bounds */
static int out_of_bounds(coord_t coords, size_t xlen, size_t ylen) {
    return (coords.x < 0 || coords.y < 0 || coords.x >= xlen || coords.y >= ylen);
}

size_t num_trails(const list_t *grid, size_t x, size_t y, size_t xlen, size_t ylen);
size_t trail_rating(const list_t *grid, size_t x, size_t y, size_t xlen, size_t ylen);

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

    /* Parse input into a grid */
    list_t grid;
    list_create(&grid, 1024, sizeof(uint8_t));
    size_t xlen;
    size_t ylen = 0;
    uint8_t num;

    for (;;) {

        /* Read a line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        ylen++;
        xlen = strlen(buffer) - 1;

        /* Parse line into row */

        for (size_t i = 0; i < xlen; i++) {
            num = buffer[i] - '0';
            list_append(&grid, &num);
        }
    }

    /* Count possible trails from a trail head */

    size_t trails = 0;
    size_t ratings = 0;
    for (size_t y = 0; y < ylen; y++) {
        for (size_t x = 0; x < xlen; x++) {
            if (deref(uint8_t, list_getindex(&grid, y * ylen + x)) == TRAILHEAD) {
                trails += num_trails(&grid, x, y, xlen, ylen);
                ratings += trail_rating(&grid, x, y, xlen, ylen);
            }
        }
    }

    printf("%lu\n", trails);
    printf("%lu\n", ratings);

    /* Close input */

    list_destroy(&grid);
    fclose(puzzle);
}

/* Recursively look for an adjacent cell that is one greater than this cell. Returns the number of reachable trail
 * ends.
 * @param grid The topological map
 * @param loc The current location
 * @param xlen The total number of columns in the topological map
 * @param ylen The total number of rows in the topological map
 * @param visited The set of previously visited trail-ends. Leave NULL to record all trail ends
 * @return The number of uniquely possible to visit trail ends
 */
static size_t look_for(const list_t *grid, coord_t loc, size_t xlen, size_t ylen, set_t *visited) {

    size_t total = 0;
    uint8_t *self = list_getindex(grid, loc.y * ylen + loc.x);

    /* This location doesn't exist */

    if (self == NULL) return 0;

    /* This location is a trail end, yippee! */

    if (*self == TRAILEND) {
        if (visited != NULL && set_contains(visited, &loc)) {
            return 0;
        } else {
            if (visited != NULL) set_add(visited, &loc);
            return 1;
        }
    }

    /* Check for any viable neighbours */
    for (size_t i = 0; i < sizeof(NEIGHBOURS) / sizeof(NEIGHBOURS[0]); i++) {

        coord_t neighbour = coord_add(loc, NEIGHBOURS[i]);

        /* Out of bounds, skip this neighbour */

        if (out_of_bounds(neighbour, xlen, ylen)) continue;

        /* If the neighbour isn't one greater than our current location, skip as well. */

        if (deref(char, list_getindex(grid, neighbour.y * ylen + neighbour.x)) != *self + 1) {
            continue;
        }

        /* The neighbour is one greater than our current value and is within bounds, go to it */

        total += look_for(grid, neighbour, xlen, ylen, visited);
    }

    return total;
}

/* Calculates the number of possible hiking trails that can be taken from the location.
 * @param grid The topological map
 * @param x The x coordinate of the location
 * @param y The y coordinate of the location
 * @param xlen The total number of columns in the topological map
 * @param ylen The total number of rows in the topological map
 * @return The total number of uniquely reachable trail ends from the location
 */
size_t num_trails(const list_t *grid, size_t x, size_t y, size_t xlen, size_t ylen) {

    /* Only trailheads are valid start positions */

    if (deref(uint8_t, list_getindex(grid, y * ylen + x)) != TRAILHEAD) {
        return 0;
    }

    /* Start looking for trail ends! But don't count previously visited trail ends. */

    set_t visited;
    set_create(&visited, NULL, 50, sizeof(coord_t));
    size_t total = look_for(grid, (coord_t){.x = x, .y = y}, xlen, ylen, &visited);
    set_destroy(&visited);

    return total;
}

/* Calculates the trail rating.
 * @param grid The topological map
 * @param x The x coordinate of the location
 * @param y The y coordinate of the location
 * @param xlen The total number of columns in the topological map
 * @param ylen The total number of rows in the topological map
 * @return The trail rating
 */
size_t trail_rating(const list_t *grid, size_t x, size_t y, size_t xlen, size_t ylen) {

    /* Only trailheads are valid start positions */

    if (deref(uint8_t, list_getindex(grid, y * ylen + x)) != TRAILHEAD) {
        return 0;
    }

    /* Start looking for trail ends! But don't count previously visited trail ends. */

    return look_for(grid, (coord_t){.x = x, .y = y}, xlen, ylen, NULL);
}
