#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

#define ROBOT '@'
#define WALL '#'
#define BOX 'O'
#define EMPTY_SPACE '.'

static char buffer[BUFSIZ];

typedef enum {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
} move_e;

typedef struct {
    int x;
    int y;
} coord_t;

const char *MOVE_STR[] = {
    [MOVE_LEFT] = "LEFT",
    [MOVE_RIGHT] = "RIGHT",
    [MOVE_UP] = "UP",
    [MOVE_DOWN] = "DOWN",
};

const coord_t MOVES[] = {
    [MOVE_LEFT] = {.x = -1, .y = 0},
    [MOVE_RIGHT] = {.x = 1, .y = 0},
    [MOVE_UP] = {.y = -1, .x = 0},
    [MOVE_DOWN] = {.x = 0, .y = 1},
};

/* Check if coordinate is out of bounds */

static int out_of_bounds(coord_t coords, size_t xlen, size_t ylen) {
    return (coords.x < 0 || coords.y < 0 || coords.x >= xlen || coords.y >= ylen);
}

void robot_move(list_t *grid, size_t xlen, size_t ylen, move_e move);

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

    /* Parse grid */

    list_t grid;
    list_create(&grid, 100, sizeof(char));

    size_t ylen = 0;
    size_t xlen = 0;
    for (;;) {

        /* Get next line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        /* Line is empty, this is the separator that indicates moves are next. */

        if (strlen(buffer) < 2) break;

        /* Otherwise append the characters */

        for (size_t i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == '\n') break;
            list_append(&grid, &buffer[i]);
        }
        ylen++;
    }
    xlen = list_len(&grid) / ylen;

    /* Parse moves */

    list_t moves;
    list_create(&moves, 100, sizeof(move_e));

    for (;;) {

        /* Get next line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) break;

        /* Add each move to the list of moves */

        for (size_t i = 0; i < strlen(buffer); i++) {
            move_e move;
            switch (buffer[i]) {
            case '<':
                move = MOVE_LEFT;
                break;
            case '>':
                move = MOVE_RIGHT;
                break;
            case '^':
                move = MOVE_UP;
                break;
            case 'v':
                move = MOVE_DOWN;
                break;
            case '\n':
                continue;
            default:
                exit(EXIT_FAILURE);
            }
            list_append(&moves, &move);
        }
    }

    /* Enact moves */

    for (size_t i = 0; i < list_len(&moves); i++) {
        move_e move = deref(move_e, list_getindex(&moves, i));
        robot_move(&grid, xlen, ylen, move);

        /* Show grid */
        for (size_t y = 0; y < ylen; y++) {
            for (size_t x = 0; x < xlen; x++) {
                printf("%c", deref(char, list_getindex(&grid, y * xlen + x)));
            }
            printf("\n");
        }
    }

    /* Close input */

    list_destroy(&grid);
    list_destroy(&moves);
    fclose(puzzle);
}

/* Try to move the robot in a direction.
 * @param grid The grid to move in (gets updated)
 * @param move The move to make (if possible)
 */
void robot_move(list_t *grid, size_t xlen, size_t ylen, move_e move) {

    const char robot = ROBOT;
    size_t i = list_index(grid, &robot);

    coord_t pos;
    pos.x = i % xlen;
    pos.y = (i - pos.x) / ylen;

    coord_t newpos = {
        .x = pos.x + MOVES[move].x,
        .y = pos.y + MOVES[move].y,
    };

    /* Check if the move can happen */
    printf("Moving %s\n", MOVE_STR[move]);

    /* Out of bounds, nuh uh */

    if (out_of_bounds(pos, xlen, ylen)) {
        return;
    }

    /* Hits a wall, not allowed */

    if (deref(char, list_getindex(grid, newpos.y * xlen + newpos.x)) == WALL) {
        return;
    }

    /* Hits an object, only allowed if there is a blank spot behind that object somewhere before a wall in the same
     * direction the robot is moving
     */

    size_t shift_loc;
    if (move == MOVE_RIGHT || move == MOVE_LEFT) {
        shift_loc = newpos.x;
    } else {
        shift_loc = newpos.y;
    }

    if (deref(char, list_getindex(grid, newpos.y * xlen + newpos.x)) == BOX) {

        switch (move) {
        case MOVE_LEFT:
            for (size_t dx = newpos.x; dx >= 0; dx--) {
                char cell = deref(char, list_getindex(grid, newpos.y * xlen + dx));
                if (cell == WALL) return; /* Wall in the way, not possible */

                /* Empty space behind obstacle, good to go */
                if (cell == EMPTY_SPACE) {
                    shift_loc = dx;
                    break;
                };
            }
            break;
        case MOVE_RIGHT:
            for (size_t dx = newpos.x; dx < xlen; dx++) {
                char cell = deref(char, list_getindex(grid, newpos.y * xlen + dx));
                if (cell == WALL) return; /* Wall in the way, not possible */

                /* Empty space behind obstacle, good to go */

                if (cell == EMPTY_SPACE) {
                    shift_loc = dx;
                    break;
                };
            }
            break;

        case MOVE_DOWN:
            for (size_t dy = newpos.y; dy < ylen; dy++) {
                char cell = deref(char, list_getindex(grid, dy * xlen + newpos.x));
                if (cell == WALL) return; /* Wall in the way, not possible */

                /* Empty space behind obstacle, good to go */
                if (cell == EMPTY_SPACE) {
                    shift_loc = dy;
                    break;
                };
            }
            break;
        case MOVE_UP:
            for (size_t dy = newpos.y; dy >= 0; dy--) {
                char cell = deref(char, list_getindex(grid, dy * xlen + newpos.x));
                if (cell == WALL) return; /* Wall in the way, not possible */

                /* Empty space behind obstacle, good to go */
                if (cell == EMPTY_SPACE) {
                    shift_loc = dy;
                    break;
                };
            }
            break;
        }
    }

    /* If we're here, the move was allowed. We have to update the robot and all the obstacles */

    char value = EMPTY_SPACE;
    list_setindex(grid, pos.y * xlen + pos.x, &value); /* Robot's last position is free now */
    printf("Shift loc %zu\n", shift_loc);

    value = BOX;
    switch (move) {

    case MOVE_LEFT:
        for (size_t i = shift_loc; i < newpos.x; i++) {
            list_setindex(grid, newpos.y * xlen + i, &value);
        }
        break;

    case MOVE_RIGHT:
        for (size_t i = shift_loc; i > newpos.x; i--) {
            list_setindex(grid, newpos.y * xlen + i, &value);
        }
        break;

    case MOVE_UP:
        for (size_t i = shift_loc; i < newpos.y; i++) {
            list_setindex(grid, i * xlen + newpos.x, &value);
        }
        break;

    case MOVE_DOWN:
        for (size_t i = shift_loc; i > newpos.y; i--) {
            list_setindex(grid, i * xlen + newpos.x, &value);
        }
        break;
    }

    value = ROBOT;
    list_setindex(grid, newpos.y * xlen + newpos.x, &robot); /* Robot moves to new position */
}
