#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/hashmap.h"
#include "../common/list.h"

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

        xlen = strlen(buffer);

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

    hmap_t antinodes;
    hmap_create(&antinodes, NULL, 1024, sizeof(coord_t), sizeof(coord_t));

    /* Iterate over all the different frequencies */

    size_t k = 0;
    list_t *antennas;
    while (hmap_iter_vals(&grid, &k, (void *)&antennas) != NULL) {

        /* Calculate the antinodes for each pair of antennas within the frequency */

        coord_t pair_antinodes[2];

        for (size_t i = 0; i < list_getlen(antennas); i++) {
            for (size_t j = 0; j < list_getlen(antennas); j++) {

                /* Skip pairs of the same antenna */
                if (i == j) continue;

                /* Calculate the antinodes for all pairs of antennas */
                calc_antinodes(list_getindex(antennas, i), list_getindex(antennas, j), pair_antinodes);

                /* If the antinodes aren't out of bounds, then record them in the set */
                for (int l = 0; l < 2; l++) {
                    if (!out_of_bounds(&pair_antinodes[l], xlen, ylen)) {
                        hmap_put(&antinodes, &pair_antinodes[l], &pair_antinodes[l]);
                    }
                }
            }
        }
    }

    printf("%lu\n", hmap_len(&antinodes));

    /* Close input */

    hmap_destroy(&grid);
    hmap_destroy(&antinodes);
    fclose(puzzle);
}

/*
 * Calculates the position of an antinode.
 * @param a Antenna A
 * @param b Antenna B
 * @param antinodes A list of two coordinate pairs representing the antinodes created by the two antennas.
 */
void calc_antinodes(const antenna_t *a, const antenna_t *b, coord_t *antinodes) {

    /* Calculate the two vector components of the distance between `a` and `b` */

    int dx = a->pos.x - b->pos.x;
    int dy = a->pos.y - b->pos.y;

    /* Antinode 1 will be an extension of these vector components from `b` */

    antinodes[0].x = b->pos.x - dx;
    antinodes[0].y = b->pos.y - dy;

    /* Antinode 2 will be a mirror of antinode 1 */

    antinodes[1].x = a->pos.x + dx;
    antinodes[1].y = a->pos.y + dy;
}
