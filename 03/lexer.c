#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void lexer_create(lexer_t *lexer, FILE *stream) { lexer->stream = stream; }

static void lexer_back(lexer_t *lexer) { ungetc(lexer->last, lexer->stream); }

static char lexer_next(lexer_t *lexer) {
    lexer->last = fgetc(lexer->stream);
    return lexer->last;
}

static bool lexer_find_mul(lexer_t *lexer) {
    static char search[] = {'m', 'u', 'l', '('};

    /* Search for 'mul(' forever */

    for (;;) {

        /* Search for starting 'm' */

        while (!feof(lexer->stream) && lexer_next(lexer) != 'm')
            ;

        if (feof(lexer->stream)) return false;

        /* Make sure all other characters are in order */

        for (size_t pos = 1; pos < sizeof(search); pos++) {

            if (lexer_next(lexer) != search[pos]) {
                return false;
            }

            if (feof(lexer->stream)) {
                return false;
            }
        }

        return true;
    }

    return false;
}

static bool lexer_parse_number(lexer_t *lexer, int *number) {
    char buffer[50] = {0};
    char c = '0';

    for (size_t i = 0; i < sizeof(buffer) - 1; i++) {
        c = lexer_next(lexer);
        if (!isdigit(c) || feof(lexer->stream)) {
            break;
        }
        buffer[i] = c;
    }

    if (feof(lexer->stream)) {
        return false;
    }

    lexer_back(lexer);

    *number = atoi(buffer);
    return true;
}

/*
 * Returns NULL if end of stream, otherwise returns a pointer to the next pair.
 */
mulpair_t *lexer_pair(lexer_t *lexer, mulpair_t *pair) {

    while (!feof(lexer->stream)) {

        /* Look for start of "mul(" */

        if (!lexer_find_mul(lexer)) {
            continue;
        }

        /* Parse number */

        if (!lexer_parse_number(lexer, &pair->a)) {
            continue;
        }

        /* Get comma */

        if (lexer_next(lexer) != ',') {
            continue;
        }

        /* Parse number */

        if (!lexer_parse_number(lexer, &pair->b)) {
            continue;
        }

        /* Get closing brace */

        if (lexer_next(lexer) != ')') {
            continue;
        }

        break;
    }

    if (feof(lexer->stream)) {
        return NULL;
    }

    return pair;
}
