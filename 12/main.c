#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"
#include "../common/set.h"

#define deref(type, thing) (*((type *)(thing)))

static char buffer[BUFSIZ];

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct {
    char type;
    size_t area;
    size_t perimeter;
    size_t sides;
} region_t;

typedef enum {
    DIR_NORTH = 0,
    DIR_SOUTH = 1,
    DIR_WEST = 2,
    DIR_EAST = 3,
} direction_e;

typedef struct {
    direction_e dir;
    coord_t pos;
} side_t;

/* Neighbouring cells represented as vectors */

static const coord_t NEIGHBOURS[] = {
    [DIR_SOUTH] = {.x = 0, .y = 1},
    [DIR_NORTH] = {.x = 0, .y = -1},
    [DIR_WEST] = {.x = -1, .y = 0},
    [DIR_EAST] = {.x = 1, .y = 0},
};

static const char *DIRSTR[] = {
    [DIR_SOUTH] = "SOUTH",
    [DIR_NORTH] = "NORTH",
    [DIR_WEST] = "WEST",
    [DIR_EAST] = "EAST",
};

/* Add two coordinates */

static coord_t coord_add(coord_t a, coord_t b) { return (coord_t){.x = a.x + b.x, .y = a.y + b.y}; }

/* Check if coordinate is out of bounds */

static int out_of_bounds(coord_t coords, size_t xlen, size_t ylen) {
    return (coords.x < 0 || coords.y < 0 || coords.x >= xlen || coords.y >= ylen);
}

void record_region(coord_t start, list_t *grid, size_t xlen, size_t ylen, list_t *registry, set_t *visited);

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

    /* Parse the puzzle input into a grid */

    list_t grid;
    list_create(&grid, 1024, sizeof(char));
    size_t xlen;
    size_t ylen = 0;

    for (;;) {

        /* Read a line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        ylen++;
        xlen = strlen(buffer) - 1;

        /* Parse line into row */

        for (size_t i = 0; i < xlen; i++) {
            list_append(&grid, &buffer[i]);
        }
    }

    /* Create regions by flood filling from a specific location
     *
     * Registry is a list of all the discovered regions
     * region -> perimeter, area, type (character)
     *
     * We also have a master set of all coordinates recorded as belonging to a region already. This way we can skip
     * recorded location when flooding.
     *
     * When we are creating a region, we create a set of all the coordinate points belonging to it during the fill
     * process.
     * area -> set length
     * Perimeter is calculated by calculating the immediate left, right, top and bottom neighbours of each cell in the
     * region. If the neighbour does not appear in the region set, then the perimeter increases by one.
     *
     * Once the perimeter calculation is done, we can free the set used to collect the region's cells, since they will
     * already be recorded in the master set of all visited cells.
     *
     * We can return the fence price after getting the area and perimeter.
     */

    list_t registry; /* Registry of all regions found so far */
    list_create(&registry, 100, sizeof(region_t));

    set_t visited; /* Master list of all cells recorded to a region already. */
    set_create(&visited, NULL, list_getlen(&grid), sizeof(coord_t));

    /* Iterate through all cells and flood from them if they haven't been recorded yet. */

    for (size_t y = 0; y < ylen; y++) {
        for (size_t x = 0; x < xlen; x++) {

            coord_t coord = {.x = x, .y = y};
            if (set_contains(&visited, &coord)) continue; /* Skip visited cells */

            /* If the cell hasn't been visited yet, record the region */

            record_region(coord, &grid, xlen, ylen, &registry, &visited);
        }
    }

    /* Print out all of the regions in the registry */

    size_t normie_price = 0;
    size_t bulk_price = 0;
    for (size_t i = 0; i < list_getlen(&registry); i++) {
        region_t *region = list_getindex(&registry, i);
        /*printf("Region(type=%c, area=%lu, perimeter=%lu)\n", region->type, region->area, region->perimeter);*/
        normie_price += region->area * region->perimeter;
        bulk_price += region->area * region->sides;
    }
    printf("%lu\n", normie_price);
    printf("%lu\n", bulk_price);

    /* Close input */

    list_destroy(&grid);
    list_destroy(&registry);
    set_destroy(&visited);
    fclose(puzzle);
}

/* Calculates the perimeter of a region.
 * @param region The cells belonging to the region
 * @param xlen The number of columns in the grid
 * @param ylen The number of rows in the grid
 * @param perimeter A set in which to store the cells belonging to the perimeter
 * @return The perimeter length of the region
 */
static size_t calculate_perimeter(set_t *region, size_t xlen, size_t ylen, set_t *perimeter) {

    /* Iterate over cells in the region and check how many adjacent cells of the same type they have. */

    size_t perim = 0;
    coord_t *cell;
    size_t i = 0;
    while (set_iter(region, &i, (void *)&cell) != NULL) {

        /* Check this cell's neighbours */

        for (size_t j = 0; j < sizeof(NEIGHBOURS) / sizeof(NEIGHBOURS[0]); j++) {
            coord_t combined = coord_add(*cell, NEIGHBOURS[j]);

            /* If the neighbour is out of bounds, then the cell has a perimeter on this side */

            if (out_of_bounds(combined, xlen, ylen)) {
                set_add(perimeter, cell);
                perim++;
                continue;
            }

            /* If the neighbour is not in the region, then the cell has a perimeter on this side */

            if (!set_contains(region, &combined)) {
                set_add(perimeter, cell);
                perim++;
                continue;
            }
        }
    }

    return perim;
}

static void flood_perim(side_t start, set_t *perimperim, set_t *visited) {

    set_add(visited, &start);

    for (size_t i = 0; i < sizeof(NEIGHBOURS) / sizeof(NEIGHBOURS[0]); i++) {
        side_t next;
        next.pos = coord_add(start.pos, NEIGHBOURS[i]);
        next.dir = start.dir;

        if (!set_contains(visited, &next) && set_contains(perimperim, &next)) {
            flood_perim(next, perimperim, visited);
        }
    }
}

/* Calculates the number of distinct sides a region has
 * @param perimeter The cells belonging to the region's perimeter
 * @return The number of distinct sides in a region
 */
static size_t calculate_sides(const set_t *perimeter, const set_t *region) {

    size_t sides = 0;

    /* Calculate the perimeter of this perimeter */

    set_t perimperim;
    set_create(&perimperim, NULL, 256, sizeof(side_t));

    coord_t *cell;
    size_t i = 0;
    while (set_iter(perimeter, &i, (void *)&cell) != NULL) {

        /* Check this cell's neighbours */

        for (size_t j = 0; j < sizeof(NEIGHBOURS) / sizeof(NEIGHBOURS[0]); j++) {
            coord_t combined = coord_add(*cell, NEIGHBOURS[j]);

            /* If the neighbour is not in the region, then the cell has a perimeter^2 on this side */

            if (!set_contains(region, &combined)) {
                side_t side = {.pos = *cell, .dir = j};
                set_add(&perimperim, &side);
                continue;
            }
        }
    }

    /* Pick a random point on the perimeter^2, flood it to find all its members. Record these members in a visited set.
     * Increment the edge count by one.
     * Then remove them all from the perimeter^2 set. Repeat until perimeter set is empty.
     */

    while (set_len(&perimperim) != 0) {

        set_t visited;
        set_create(&visited, NULL, 256, sizeof(side_t));

        size_t start_i = 0;
        side_t *start = set_iter(&perimperim, &start_i, NULL);

        flood_perim(*start, &perimperim, &visited);

        /* Remove all visited cells */

        start_i = 0;
        while (set_iter(&visited, &start_i, (void *)&start) != NULL) {
            set_remove(&perimperim, start);
        }

        sides++; /* This means there's been another side */

        /* Destroy visited set for it to be reborn. */

        set_destroy(&visited);
    }

    set_destroy(&perimperim);

    return sides;
}

/* Floods a region from a starting point, recording all of the coordinates visited.
 * @param start The location to start flooding from
 * @param grid The map where the cells are stored
 * @param xlen The number of columns in the grid
 * @param ylen The number of rows in the grid
 * @param visited The set of all visited locations
 * @param region The set to store the cells belonging to this region
 */
static void flood_region(coord_t start, list_t *grid, size_t xlen, size_t ylen, set_t *visited, set_t *region) {

    char type = deref(char, list_getindex(grid, start.y * ylen + start.x));

    /* Record the start location as visited */

    set_add(visited, &start);
    set_add(region, &start);

    /* Start flooding from each unvisited neighbour of the correct type */

    for (size_t i = 0; i < sizeof(NEIGHBOURS) / sizeof(NEIGHBOURS[0]); i++) {

        coord_t combined = coord_add(start, NEIGHBOURS[i]);

        /* If the neighbour is out of bounds, skip it */

        if (out_of_bounds(combined, xlen, ylen)) {
            continue;
        }

        /* If the neighbour is not of the right type, then skip it */

        if (deref(char, list_getindex(grid, combined.y * ylen + combined.x)) != type) {
            continue;
        }

        /* If the neighbour has already been recorded, skip it
         * NOTE: assumption made to only check against the region set because it should be impossible for the visited
         * set to contain anything that the region set doesn't at this stage.
         */

        if (set_contains(region, &combined)) {
            continue;
        }

        /* We found an unvisited cell belonging to this region! Flood from there. */

        flood_region(combined, grid, xlen, ylen, visited, region);
    }

    return;
}

/* Records a new region starting at `start`. The region is added to the registry and all its cells are recorded in the
 * `visited` set.
 * @param start The location to start flooding from
 * @param grid The map where the cells are stored
 * @param xlen The number of columns in the grid
 * @param ylen The number of rows in the grid
 * @param registry The register of all regions
 * @param visited The set of all visited locations
 */
void record_region(coord_t start, list_t *grid, size_t xlen, size_t ylen, list_t *registry, set_t *visited) {

    /* Sanity check, don't record this region if we've visited it */

    if (set_contains(visited, &start)) return;

    /* Start flooding from the start location and fill up the set of cells belonging to the region */

    set_t region_cells;
    set_create(&region_cells, NULL, 2048, sizeof(coord_t));

    /* Flood out the region! */

    flood_region(start, grid, xlen, ylen, visited, &region_cells);

    /* Calculate the perimeter of the region */

    set_t perimeter;
    set_create(&perimeter, NULL, 1024, sizeof(coord_t));
    region_t region = {
        .area = set_len(&region_cells),
        .perimeter = calculate_perimeter(&region_cells, xlen, ylen, &perimeter),
        .sides = calculate_sides(&perimeter, &region_cells),
        .type = deref(char, list_getindex(grid, start.y * ylen + start.x)),
    };

    /* Free the coordinates now that we're done with them */

    set_destroy(&perimeter);
    set_destroy(&region_cells);

    /* Add the new region to the registry */

    list_append(registry, &region);
}
