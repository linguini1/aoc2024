#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) *((type *)((thing)))
#define array_size(arr) (sizeof(arr) / sizeof(arr[0]))

unsigned int xmas_count(list_t *grid, size_t xlen, size_t ylen, size_t x, size_t y, char find);
char next_char(char cur);
int is_xmas(list_t *grid, size_t xlen, size_t ylen, size_t x, size_t y);

typedef struct {
    int x;
    int y;
} coord_t;

/* Surrounding cell relative vectors */
const coord_t SURROUNDING[] = {
    {0, 1}, {1, 0}, {1, 1}, {0, -1}, {-1, 0}, {-1, -1}, {1, -1}, {-1, 1},
};

const coord_t DIAGONAL[] = {
    /* First diagonal */
    {1, 1},
    {-1, -1},
    /* Second diagonal */
    {1, -1},
    {-1, 1},
};

int main(int argc, char **argv) {

    if (argc != 2) {
        fprintf(stderr, "Provide the name of the file to use as puzzle input.");
        return EXIT_FAILURE;
    }

    /* Open the puzzle input */

    FILE *puzzle = fopen(argv[1], "r");
    if (puzzle == NULL) {
        fprintf(stderr, "Failed to open puzzle input file '%s': %s\n", argv[1], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /*
     * Approach: start on each letter X, since that is the beginning of the word "XMAS".
     * Choose all characters in the surrounding 7 locations that is an "M" to go to next.
     * Choose all characters in the surrounding 7 locations that is an "A" to go to next.
     *
     * Perform this recursively to find all instances of the word XMAS.
     */

    /* Load the input into a list, which will be treated like a 2D array */

    list_t grid;
    list_create(&grid, 100, sizeof(char));

    char c;
    size_t ylen = 0; /* Length of a column (y direction) */
    while (!feof(puzzle)) {

        c = fgetc(puzzle);

        /* Skip newlines and EOF. */

        if (c == '\n' || c == EOF) {
            ylen++;
            continue;
        }

        /* Append character to grid */

        list_append(&grid, &c);
    }
    ylen--; /* Subtract extra count for EOF */
    size_t xlen = list_getlen(&grid) / ylen;

    /* Recursively count occurrences of the word XMAS from each 'X' */

    size_t total = 0;
    for (size_t y = 0; y < ylen; y++) {
        for (size_t x = 0; x < xlen; x++) {
            if (deref(char, list_getindex(&grid, y * ylen + x)) == 'X') {
                total += xmas_count(&grid, xlen, ylen, x, y, next_char('X'));
            }
        }
    }

    printf("%lu\n", total);

    /* Look for X-MAS */

    size_t cross_total = 0;
    for (size_t y = 0; y < ylen; y++) {
        for (size_t x = 0; x < xlen; x++) {
            if (deref(char, list_getindex(&grid, y * ylen + x)) == 'A') {
                cross_total += is_xmas(&grid, xlen, ylen, x, y);
            }
        }
    }

    printf("%lu\n", cross_total);

    /* Close input */

    list_destroy(&grid);
    fclose(puzzle);
}

char next_char(char cur) {

    switch (cur) {
    case 'X':
        return 'M';
    case 'M':
        return 'A';
    case 'A':
        return 'S';
    default:
        return '0';
    }
}

static int out_of_bounds(int x, int y, size_t xlen, size_t ylen) {
    return (x < 0 || y < 0) || ((x >= xlen) || (y >= ylen));
}

static unsigned int _xmas_count_rec(list_t *grid, size_t xlen, size_t ylen, size_t x, size_t y, char find,
                                    const coord_t *dir) {
    int next_x = x + dir->x;
    int next_y = y + dir->y;

    /* Out of bounds */

    if (out_of_bounds(next_x, next_y, xlen, ylen)) {
        return 0;
    }

    /* Check if the grid matches the character being searched for */

    if (deref(char, list_getindex(grid, next_y * ylen + next_x)) == find) {

        /* If this is the end of the word "XMAS" then log a success */
        if (next_char(find) == '0') {
            return 1;
        }

        /* Otherwise, search for the next character in the same direction */
        return _xmas_count_rec(grid, xlen, ylen, next_x, next_y, next_char(find), dir);
    }

    return 0;
}

unsigned int xmas_count(list_t *grid, size_t xlen, size_t ylen, size_t x, size_t y, char find) {

    /* Search all surrounding squares for the next letter if the direction is not specified
     * Otherwise, only search in that direction.
     */

    size_t total = 0;

    for (size_t i = 0; i < array_size(SURROUNDING); i++) {
        total += _xmas_count_rec(grid, xlen, ylen, x, y, find, &SURROUNDING[i]);
    }

    return total;
}

/* Returns 0 if false, 1 otherwise. Checks for MS on both diagonals. NOTE: Assumes passed x,y coordinate is definitely
 * an A.
 */
int is_xmas(list_t *grid, size_t xlen, size_t ylen, size_t x, size_t y) {

    int next_x;
    int next_y;
    char c1;
    char c2;

    /* If any coordinate is out of bounds then we fail */
    /* If the cell is not an 'M' or 'S' then we fail */
    /* If the opposite diagonal cell is not the opposite letter, we fail */

    for (size_t i = 0; i < 3; i += 2) {
        next_x = x + DIAGONAL[i].x;
        next_y = y + DIAGONAL[i].y;
        if (out_of_bounds(next_x, next_y, xlen, ylen)) return 0;
        c1 = deref(char, list_getindex(grid, next_y * ylen + next_x));
        if (c1 != 'M' && c1 != 'S') return 0;

        next_x = x + DIAGONAL[i + 1].x;
        next_y = y + DIAGONAL[i + 1].y;
        if (out_of_bounds(next_x, next_y, xlen, ylen)) return 0;
        c2 = deref(char, list_getindex(grid, next_y * ylen + next_x));
        if ((c2 != 'M' && c2 != 'S') || c1 == c2) return 0;
    }

    return 1;
}
