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

    /* Compact the files */

    list_t compacted;
    list_create(&compacted, 60, sizeof(file_t));
    list_append(&compacted, list_getindex(&files, 0)); /* First file goes right in the compacted format */

    file_t tail = {0};
    file_t *head;
    size_t last = 0;
    for (;;) {

        /* Get the next file off the end of the list if we're done with the previous tail */
        if (tail.size == 0) {
            list_pop(&files, &tail);
        }

        /* Get the last file in the compacted list */
        head = list_getindex(&compacted, list_getlen(&compacted) - 1);

        /* If there is no free space at the end of this file, we have to go to the next file to see if it has free space
         * that we can use */
        while (head->freespace == 0) {
            last++;

            /* We've compacted everything */
            if (last == tail.id) {
                head->size += tail.size;
                break;
            }

            head = list_getindex(&files, last);
            if (head == NULL) break;
            list_append(&compacted, head);
            head = list_getindex(&compacted, list_getlen(&compacted) - 1); /* Use our copy */
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
            list_append(&compacted, &newfile);
        }
    }

    /* Calculate checksum */

    printf("%llu\n", checksum(&compacted));

    /* Close input */

    list_destroy(&files);
    list_destroy(&compacted);
    fclose(puzzle);
}

/* Calculates the checksum for the file system
 * @param filesystem The filesystem to calculate the checksum for
 * @return The checksum
 */
size_t checksum(const list_t *filesystem) {
    size_t checksum = 0;
    for (size_t i = 0, pos = 0; i < list_getlen(filesystem); i++) {
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
