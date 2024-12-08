#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    /* Create lexer */

    lexer_t lexer;
    lexer_create(&lexer, puzzle);

    /* Lex out pairs */

    mulpair_t pair;
    size_t sum = 0;

    while (lexer_pair(&lexer, &pair) != NULL) {
        sum += pair.a * pair.b;
    }

    printf("%lu\n", sum);

    return 0;
}
