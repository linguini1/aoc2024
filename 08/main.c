#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/hashmap.h"
#include "../common/list.h"
#include "../common/set.h"

#define EMPTY_CELL '.'
#define deref(type, thing) (*((type *)(thing)))

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct {
    coord_t pos;
    char freq;
} antenna_t;

static char buffer[BUFSIZ];

static int out_of_bounds(coord_t *coord, size_t xlen, size_t ylen) {
    return coord->x < 0 || coord->y < 0 || coord->x >= xlen || coord->y >= ylen;
}

void calc_antinodes(const antenna_t *a, const antenna_t *b, coord_t *antinodes);

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

    /* Parse input into antenna locations with their frequencies */

    hmap_t grid;
    hmap_create(&grid, NULL, 256, sizeof(char), sizeof(list_t));
    size_t ylen = 0;
    size_t xlen;

    for (;;) {

        /* Get next line of puzzle input */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        xlen = strlen(buffer) - 1;

        /* Check each row for antennas and store their location and frequency */

        antenna_t antenna;
        for (size_t i = 0; i < strlen(buffer); i++) {

            /* Skip empty cells */
            if (buffer[i] == EMPTY_CELL || buffer[i] == '\n') {
                continue;
            }

            /* Construct antenna */
            antenna.pos.x = i;
            antenna.pos.y = ylen;
            antenna.freq = buffer[i];

            /* Check if a list of this frequency already exists */

            list_t *freq_list = hmap_get(&grid, &antenna.freq);
            if (freq_list != NULL) {
                list_append(freq_list, &antenna);
            } else {
                /* No list exists already, create one */
                list_t newlist;
                list_create(&newlist, 5, sizeof(antenna_t));
                list_append(&newlist, &antenna);
                hmap_put(&grid, &antenna.freq, &newlist);
            }
        }

        ylen++;
    }

    /* Create a set for the unique antinode locations */

    set_t antinodes;
    set_create(&antinodes, NULL, 1024, sizeof(coord_t));

    set_t antinodes_all;
    set_create(&antinodes_all, NULL, 2048, sizeof(coord_t));

    /* Iterate over all the different frequencies */

    size_t k = 0;
    list_t *antennas;
    while (hmap_iter_vals(&grid, &k, (void *)&antennas) != NULL) {

        /* Calculate the antinodes for each pair of antennas within the frequency */

        coord_t antipair[2];

        for (size_t i = 0; i < list_getlen(antennas); i++) {
            for (size_t j = 0; j < list_getlen(antennas); j++) {

                /* Skip pairs of the same antenna */
                if (i == j) continue;

                antenna_t *a = list_getindex(antennas, i);
                antenna_t *b = list_getindex(antennas, j);

                /* Calculate the antinodes for all pairs of antennas */

                int dx = a->pos.x - b->pos.x;
                int dy = a->pos.y - b->pos.y;
                int i = 0;

                for (;;) {

                    /* Antinode 1 will be an extension of these vector components from `b` */

                    antipair[0].x = b->pos.x - dx * i;
                    antipair[0].y = b->pos.y - dy * i;

                    /* Antinode 2 will be a mirror of antinode 1 */

                    antipair[1].x = a->pos.x + dx * i;
                    antipair[1].y = a->pos.y + dy * i;

                    /* If we're calculating outside the perimeter entirely, we can move on */
                    if (out_of_bounds(&antipair[0], xlen, ylen) && out_of_bounds(&antipair[1], xlen, ylen)) {
                        break;
                    }

                    /* Only record the first pair of antinodes for part 1 */
                    if (i == 1) {
                        if (!out_of_bounds(&antipair[0], xlen, ylen)) set_add(&antinodes, &antipair[0]);
                        if (!out_of_bounds(&antipair[1], xlen, ylen)) set_add(&antinodes, &antipair[1]);
                    }

                    /* Record all antinodes for part 2 */
                    if (!out_of_bounds(&antipair[0], xlen, ylen)) set_add(&antinodes_all, &antipair[0]);
                    if (!out_of_bounds(&antipair[1], xlen, ylen)) set_add(&antinodes_all, &antipair[1]);

                    i++;
                }
            }
        }
    }

    printf("%lu\n", set_len(&antinodes));
    printf("%lu\n", set_len(&antinodes_all));

    /* Close input */

    hmap_destroy(&grid);
    set_destroy(&antinodes);
    set_destroy(&antinodes_all);
    fclose(puzzle);
}
