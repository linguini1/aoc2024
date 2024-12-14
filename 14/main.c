#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

#define XLEN 101
#define YLEN 103
#define SECONDS 100

typedef struct {
    int x;
    int y;
} coord_t;

typedef struct {
    coord_t pos;
    coord_t vel;
} robot_t;

static char buffer[BUFSIZ];

/* Add two coordinates */

static coord_t coord_add(coord_t a, coord_t b) {
    coord_t coord = {
        .x = (a.x + b.x) % XLEN,
        .y = (a.y + b.y) % YLEN,
    };
    if (coord.x < 0) {
        coord.x += XLEN;
    }
    if (coord.y < 0) {
        coord.y += YLEN;
    }
    return coord;
}

/* Check if coordinate is out of bounds */

static int out_of_bounds(coord_t coords) {
    return (coords.x < 0 || coords.y < 0 || coords.x >= XLEN || coords.y >= YLEN);
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

    /* Parse the input into robot positions and velocities */

    list_t robots;
    list_create(&robots, 128, sizeof(robot_t));

    char *tok;
    robot_t cur;
    for (;;) {

        /* Get next input line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        /* Parse out position */

        tok = strtok(buffer, "=");
        tok = strtok(NULL, ",");
        cur.pos.x = atoi(tok);
        tok = strtok(NULL, " ");
        cur.pos.y = atoi(tok);

        tok = strtok(NULL, "=");
        tok = strtok(NULL, ",");
        cur.vel.x = atoi(tok);
        tok = strtok(NULL, " ");
        cur.vel.y = atoi(tok);

        /* Append to list of robots */

        list_append(&robots, &cur);
    }

    /* Each second, move the robots */

    for (size_t t = 0; t < SECONDS; t++) {

        /* Iterate over each robot and update its position */

        for (size_t i = 0; i < list_len(&robots); i++) {
            robot_t *robot = list_getindex(&robots, i);
            coord_t newpos = coord_add(robot->pos, robot->vel);
            robot->pos = newpos;
        }
    }

    /* Check how many robots are in each quadrant */

    size_t quadrants[4] = {0, 0, 0, 0};

    for (size_t i = 0; i < list_len(&robots); i++) {
        robot_t *robot = list_getindex(&robots, i);

        /* Skip robots right on the center line */

        if (robot->pos.x == (XLEN / 2)) continue;
        if (robot->pos.y == (YLEN / 2)) continue;

        /* First half */

        if (robot->pos.x < (XLEN / 2)) {

            /* Bottom */
            if (robot->pos.y > (YLEN / 2)) {
                quadrants[0]++;
            }

            /* Top */
            else {
                quadrants[2]++;
            }

        }

        /* Second half */

        else {

            /* Bottom */

            if (robot->pos.y > (YLEN / 2)) {
                quadrants[3]++;
            }

            /* Top */

            else {
                quadrants[1]++;
            }
        }
    }

    printf("%lu\n", quadrants[0] * quadrants[1] * quadrants[2] * quadrants[3]);

    /* Close input */

    list_destroy(&robots);
    fclose(puzzle);
}
