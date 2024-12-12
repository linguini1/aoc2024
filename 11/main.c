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

void blink(hmap_t *stones, hmap_t *recipes);
void counter_incr_or_create(hmap_t *counter, stone_t *key, size_t val);

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

    hmap_t stones;
    hmap_create(&stones, NULL, 1024, sizeof(stone_t), sizeof(size_t));

    stone_t cur;
    for (;;) {

        /* Get next line */

        if (fgets(buffer, sizeof(buffer), puzzle) == NULL) {
            break;
        }

        char *tok = strtok(buffer, " ");
        do {
            cur = strtoul(tok, NULL, 10);

            /* Increase counter of this stone type */

            counter_incr_or_create(&stones, &cur, 1);
            tok = strtok(NULL, " ");
        } while (tok != NULL);
    }

    /* Create a hashmap of recipes to cache what each rock's evolution is */

    hmap_t recipes;
    hmap_create(&recipes, NULL, 2048, sizeof(stone_t), sizeof(recipe_t));

    for (size_t i = 0; i < 4; i++) {
        blink(&stones, &recipes);
    }

    /* Add up all the counter values */

    size_t total = 0;
    size_t j = 0;
    size_t *count;
    stone_t *stone;

    while (hmap_iter_pairs(&stones, &j, (void *)&stone, (void *)&count) != NULL) {
        if (*count != 0) printf("Stone %lu: %lu\n", *stone, *count);
        total += *count;
    }

    printf("%lu\n", total);

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

/* Increment the counter associated with `key` if it exists, or start a new counter with initial value `val`.
 * @param counter The hashmap counter
 * @param key The key to increment the count for
 * @param val The amount to increment by
 */
void counter_incr_or_create(hmap_t *counter, stone_t *key, size_t val) {
    size_t *count = hmap_get(counter, key);
    if (count == NULL) {
        hmap_put(counter, key, &val);
    } else {
        *count += val;
    }
}

/* Calculate the result of a blink and update the list.
 * @param stones The current roster of stones
 * @param recipes A place to store stone's states of evolution as they are discovered
 */
void blink(hmap_t *stones, hmap_t *recipes) {

    /* Get a copy of the entries for this iteration */

    list_t key_list;
    list_create(&key_list, 100, sizeof(stone_t));

    list_t count_list;
    list_create(&count_list, 100, sizeof(size_t));

    size_t k = 0;
    stone_t *key;
    size_t *old_count;
    while (hmap_iter_pairs(stones, &k, (void *)&key, (void *)&old_count)) {
        if (*old_count != 0) {
            list_append(&key_list, key);
            list_append(&count_list, old_count);
        }
    }

    printf("Round keys:\n");
    for (size_t i = 0; i < list_getlen(&key_list); i++) {
        printf("(%lu, %lu), ", deref(stone_t, list_getindex(&key_list, i)),
               deref(size_t, list_getindex(&count_list, i)));
    }
    printf("\n");

    /* Iterate over all stones in this list and apply the rules */

    stone_t *cur;
    size_t *count;
    size_t *round_count;

    for (size_t i = 0; i < list_getlen(&key_list); i++) {

        cur = list_getindex(&key_list, i);
        round_count = list_getindex(&count_list, i);
        count = hmap_get(stones, cur);
        if (*count == 0) continue; /* Skip stones that we have seen but aren't in this round */

        /* Check if we already know the outcome of this stone's evolution */

        recipe_t *recipe = hmap_get(recipes, cur);
        if (recipe != NULL) {

            *count = *count - *round_count; /* All stones just converted */

            /* Record more of this stone's replacement */

            counter_incr_or_create(stones, &recipe->replace, *round_count);

            /* If this recipe calls for a new stone, add it */

            counter_incr_or_create(stones, &recipe->add, *round_count);

            continue;
        }

        /* If the stone is engraved with the number 0, it is replaced by a stone engraved with the number 1. */

        if (*cur == 0) {

            /* Update the recipe book */

            recipe_t newrecipe = {.replace = 1, .dual = false};
            hmap_put(recipes, cur, &newrecipe);

            /* Update the counter */

            *count = *count - *round_count; /* All stones just converted */

            /* All 0 stones became '1' stones */

            counter_incr_or_create(stones, &newrecipe.replace, *round_count);

            continue;
        }

        /* If the stone is engraved with a number that has an even number of digits, it is replaced by two stones.
         * The left half of the digits are engraved on the new left stone, and the right half of the digits are
         * engraved on the new right stone. (The new numbers don't keep extra leading zeroes: 1000 would become
         * stones 10 and 0.) */

        if (num_digits(*cur) % 2 == 0) {

            stone_t prev_cur = *cur; /* Save for recipe book */

            /* Calculate the two halves */

            recipe_t newrecipe = {.dual = true};
            newrecipe.replace = *cur;
            split_stone(&newrecipe.replace, &newrecipe.add);

            /* Update the recipe book */

            hmap_put(recipes, &prev_cur, &newrecipe);
            *count = *count - *round_count; /* All stones just converted */

            /* Increase the count of the first half */

            counter_incr_or_create(stones, &newrecipe.replace, *round_count);

            /* Increase the count of the second half */

            counter_incr_or_create(stones, &newrecipe.add, *round_count);

            continue;
        }

        /* If none of the other rules apply, the stone is replaced by a new stone; the old stone's number multiplied
         * by 2024 is engraved on the new stone. */

        /* Update recipe book */

        size_t prev = *cur;
        recipe_t newrecipe = {.replace = prev * 2024, .dual = false};
        hmap_put(recipes, &prev, &newrecipe);

        *count = *count - *round_count; /* All stones just converted */

        /* Increase the count of the replacement stone */

        counter_incr_or_create(stones, &newrecipe.replace, *round_count);
    }

    /* Destroy the count list */

    list_destroy(&key_list);
    list_destroy(&count_list);
}
