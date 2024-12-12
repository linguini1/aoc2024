#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/hashmap.h"
#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))
#define NUM_BLINKS 25
#define NUM_MORE_BLINKS 75

typedef size_t stone_t;

/* A recipe for a stone's evolution */
typedef struct {
    stone_t replace; /* A stone to replace the current stone with */
    stone_t add;     /* A new stone to add to the list */
    bool dual;       /* If the stone's add field is valid */
} recipe_t;

static char buffer[BUFSIZ];

void blink(list_t *stones, hmap_t *recipes);

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

    /* Parse input into stones */

    list_t stones;
    list_create(&stones, 1024, sizeof(stone_t));

    stone_t cur;
    for (;;) {

        /* Get next line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        char *tok = strtok(buffer, " ");
        do {
            cur = strtoul(tok, NULL, 10);
            list_append(&stones, &cur);
            tok = strtok(NULL, " ");
        } while (tok != NULL);
    }

    /* Create a hashmap of recipes to cache what each rock's evolution is */

    hmap_t recipes;
    hmap_create(&recipes, NULL, 2048, sizeof(stone_t), sizeof(recipe_t));

    for (size_t i = 0; i < NUM_BLINKS; i++) {
        blink(&stones, &recipes);
    }

    printf("%lu\n", list_getlen(&stones));

    for (size_t i = 0; i < NUM_MORE_BLINKS - NUM_BLINKS; i++) {
        printf("Blink #%lu\n", i);
        blink(&stones, &recipes);
    }

    printf("%lu\n", list_getlen(&stones));

    /* Close input */

    fclose(puzzle);
}

/* Returns the number of digits in a number represented in base 10
 * @param num The number to check the digits of
 * @return The number of digits in `num` if represented in base 10
 */
static size_t num_digits(size_t num) {
    size_t digits = 0;

    while (num > 0) {
        num /= 10;
        digits++;
    }

    return digits;
}

/* Splits a stone in two.
 * @param stone The stone to split in two
 * @param remainder A reference to the stone in which to store the remainder of the split
 */
static void split_stone(stone_t *stone, stone_t *remainder) {
    size_t digits = num_digits(*stone);
    assert(digits % 2 == 0);

    size_t first_half = *stone / pow(10, digits / 2);
    *remainder = *stone - first_half * pow(10, digits / 2);
    *stone = first_half;
}

/* Calculate the result of a blink and update the list.
 * @param stones The current roster of stones
 * @param recipes A place to store stone's states of evolution as they are discovered
 */
void blink(list_t *stones, hmap_t *recipes) {

    size_t len = list_getlen(stones);

    /* Iterate over all stones and apply the rules */

    stone_t *cur;
    for (size_t i = 0; i < len; i++) {
        cur = list_getindex(stones, i);

        /* Check if we already know the outcome of this stone's evolution */

        recipe_t *recipe = hmap_get(recipes, cur);
        if (recipe != NULL) {
            *cur = recipe->replace; /* Always update the current stone */
            /* If this recipe calls for a new stone, add it */
            if (recipe->dual) list_append(stones, &recipe->add);
            continue;
        }

        /* If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1. */

        if (*cur == 0) {

            /* Update the recipe book */
            recipe_t newrecipe = {.replace = 1, .dual = false};
            hmap_put(recipes, cur, &newrecipe);

            *cur = 1;
            continue;
        }

        /* If the stone is engraved with a number that has an even number of digits, it is replaced by two stones. The
         * left half of the digits are engraved on the new left stone, and the right half of the digits are engraved on
         * the new right stone. (The new numbers don't keep extra leading zeroes: 1000 would become stones 10 and 0.) */

        if (num_digits(*cur) % 2 == 0) {

            stone_t prev_cur = *cur; /* Save for recipe book */

            /* Calculate the two halves */

            stone_t second_stone;
            split_stone(cur, &second_stone);

            /* Update the recipe book */

            recipe_t newrecipe = {.replace = *cur, .add = second_stone, .dual = true};
            hmap_put(recipes, &prev_cur, &newrecipe);

            /* Add the new half to the list */

            list_append(stones, &second_stone); /* Add the new stone */
            continue;
        }

        /* If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied by
         * 2024 is engraved on the new stone. */

        *cur *= 2024;
    }
}
