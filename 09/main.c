#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/list.h"

#define deref(type, thing) (*((type *)(thing)))

typedef struct {
    size_t id;
    uint8_t size;
    uint8_t freespace;
} file_t;

size_t checksum(const list_t *filesystem);
void fine_grain_compact(const list_t *og_files, list_t *compacted);
void coarse_grain_compact(const list_t *og_files, list_t *compacted);

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

    /* Populate a list of files */

    list_t files;
    list_create(&files, 50, sizeof(file_t));

    char buf;
    file_t file;
    for (file.id = 0;; file.id++) {

        /* Parse out file size (char to int) */

        buf = fgetc(puzzle);
        if (!isdigit(buf)) break; /* Done */
        file.size = buf - '0';

        /* Parse out file size (char to int) */

        buf = fgetc(puzzle);
        if (!isdigit(buf)) {
            file.freespace = 0;
        } else {
            /* Assume no free space after file if it is the last one in the list */
            file.freespace = buf - '0';
        }

        /* Append file to list of files */

        list_append(&files, &file);
    }

    /* Calculate checksum for fine-grain compacted file system */

    list_t fine_grain;
    fine_grain_compact(&files, &fine_grain);
    printf("%llu\n", checksum(&fine_grain));
    list_destroy(&fine_grain);

    /* Calculate checksum for coarse-grain compacted file system */

    list_t coarse_grain;
    coarse_grain_compact(&files, &coarse_grain);
    printf("%llu\n", checksum(&coarse_grain));
    list_destroy(&coarse_grain);

    /* Close input */

    list_destroy(&files);
    fclose(puzzle);
}

/* Compacts the filesystem on a per-file basis (coarse-grained).
 * @param og_files The file system to compact
 * @param compacted A pointer to an uninitialized list where the compacted file system can be stored. Caller's
 * responsibility to free.
 */
void coarse_grain_compact(const list_t *og_files, list_t *compacted) {

    /* Create a copy of the original files so we can modify them */

    list_create(compacted, list_len(og_files), sizeof(file_t));
    for (size_t i = 0; i < list_len(og_files); i++) {
        list_append(compacted, list_getindex(og_files, i));
    }

    /* Compact the files. Now we consider the index into the list to be the position of the file */

    for (size_t i = list_len(og_files) - 1; i > 0; i--) {

        /* Get the highest ID file and try to move it */

        file_t *to_move;
        size_t taken_from = 0;
        for (taken_from = 0; taken_from < list_len(compacted); taken_from++) {

            /* Go through the list until we find the file ID we're looking for */
            to_move = list_getindex(compacted, taken_from);
            if (to_move->id == i) {
                break;
            }
        }

        /* Go through current compacted file system state and look for somewhere with room to move to */

        for (size_t j = 0; j < list_len(compacted); j++) {

            /* Get the next file slot */

            file_t *cur = list_getindex(compacted, j);
            if (cur->id == to_move->id) break; /* We reached ourselves, nothing earlier is free */

            /* The file has enough free space after it for us to move to */

            if (cur->freespace >= to_move->size) {

                /* Figure out the file right before the file to be moved */

                file_t *prev = list_getindex(compacted, list_index(compacted, to_move) - 1);
                prev->freespace += (to_move->size + to_move->freespace);

                cur->freespace -= to_move->size; /* Decrease free space by the amount this file occupies */

                /* This file now inherits the trailing free space */

                to_move->freespace = cur->freespace;
                cur->freespace = 0;

                /* Update this file's position to be right after `cur` by shifting down all subsequent files. */

                file_t temp =
                    deref(file_t, list_getindex(compacted, j + 1)); /* Get the file that was right after cur */
                list_setindex(compacted, j + 1, to_move);           /* Put this file right after `cur` */

                /* Shift down files */

                for (size_t k = j + 2; k <= taken_from; k++) {
                    file_t temp2 = deref(file_t, list_getindex(compacted, k)); /* Get k */
                    list_setindex(compacted, k, &temp);                        /* Store k-1 in k */
                    temp = temp2;
                }

                break;
            }
        }
    }
}

/* Compacts the filesystem on a per-block basis (fine-grained).
 * @param og_files The file system to compact
 * @param compacted A pointer to an uninitialized list where the compacted file system can be stored. Caller's
 * responsibility to free.
 */
void fine_grain_compact(const list_t *og_files, list_t *compacted) {

    /* Create a copy of the original files so we can modify them */

    list_t cpfiles;
    list_create(&cpfiles, list_len(og_files), sizeof(file_t));
    for (size_t i = 0; i < list_len(og_files); i++) {
        list_append(&cpfiles, list_getindex(og_files, i));
    }

    /* Compact the files */

    list_create(compacted, 60, sizeof(file_t));
    list_append(compacted, list_getindex(&cpfiles, 0)); /* First file goes right in the compacted format */

    file_t tail = {0};
    file_t *head;
    size_t last = 0;
    for (;;) {

        /* Get the next file off the end of the list if we're done with the previous tail */
        if (tail.size == 0) {
            list_pop(&cpfiles, &tail);
        }

        /* Get the last file in the compacted list */
        head = list_getindex(compacted, list_len(compacted) - 1);

        /* If there is no free space at the end of this file, we have to go to the next file to see if it has free space
         * that we can use */
        while (head->freespace == 0) {
            last++;

            /* We've compacted everything */
            if (last == tail.id) {
                head->size += tail.size;
                break;
            }

            head = list_getindex(&cpfiles, last);
            if (head == NULL) break;
            list_append(compacted, head);
            head = list_getindex(compacted, list_len(compacted) - 1); /* Use our copy */
        }
        if (head == NULL) break;

        /* We've compacted everything */
        if (last == tail.id) break;

        /* If the files are the same, just increase the file size of the file in the compacted list until the freespace
         * has run out */
        if (head->id == tail.id) {
            while (head->freespace > 0 && tail.size > 0) {
                tail.size--;       /* Take block from tail */
                head->freespace--; /* Add block to head (less free space) */
                head->size++;      /* Add block to head (more size) */
            }
        }

        /* Otherwise the files aren't the same, we need to append a new file to the compacted list
         * The file with the new ID inherits the remaining free space of the previous file.
         */
        else {
            head->freespace--; /* Space for the first block of the new file */
            tail.size--;       /* Take one block off the tail for the new file */
            file_t newfile = {.id = tail.id, .freespace = head->freespace, .size = 1};
            head->freespace = 0; /* This free space has been inherited by the new file */
            list_append(compacted, &newfile);
        }
    }

    /* Get rid of the copy */

    list_destroy(&cpfiles);
}

/* Calculates the checksum for the file system
 * @param filesystem The filesystem to calculate the checksum for
 * @return The checksum
 */
size_t checksum(const list_t *filesystem) {
    size_t checksum = 0;
    for (size_t i = 0, pos = 0; i < list_len(filesystem); i++) {
        file_t *cur = list_getindex(filesystem, i);

        /* Add up each block */

        for (size_t j = 0; j < cur->size; j++) {
            checksum += cur->id * pos;
            pos++;
        }

        /* Empty spaces count for nothing but contribute to position */
        pos += cur->freespace;
    }
    return checksum;
}
