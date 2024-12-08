#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* Create a new lexer */
void lexer_create(lexer_t *lexer, FILE *stream) {
    lexer->stream = stream;
    lexer->apply = true;
}

/* Whether the previous multiplication is applicable or not */
bool lexer_applicable(lexer_t *lexer) { return lexer->apply; }

/* Go back a character */
static void lexer_back(lexer_t *lexer) { ungetc(lexer->last, lexer->stream); }

/* Go forward a character */
static char lexer_next(lexer_t *lexer) {
    lexer->last = fgetc(lexer->stream);
    return lexer->last;
}

/* Find the string 'mul(' */
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

/* Parse a number */
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

static bool lexer_multiplication(lexer_t *lexer, mulpair_t *pair) {

    /* Look for start of "mul(" */

    if (!lexer_find_mul(lexer)) {
        return false;
    }

    /* Parse number */

    if (!lexer_parse_number(lexer, &pair->a)) {
        return false;
    }

    /* Get comma */

    if (lexer_next(lexer) != ',') {
        return false;
    }

    /* Parse number */

    if (!lexer_parse_number(lexer, &pair->b)) {
        return false;
    }

    /* Get closing brace */

    if (lexer_next(lexer) != ')') {
        return false;
    }

    /* If we're not at the end of the file then we found a multiplication instruction */

    return !feof(lexer->stream);
}

/*
 * Returns NULL if end of stream, otherwise returns a pointer to the next pair.
 */
mulpair_t *lexer_pair(lexer_t *lexer, mulpair_t *pair) {

    while (!feof(lexer->stream)) {

        lexer_next(lexer); /* Get the next character */

        /* If there is an 'm', look for a multiplication instruction. */

        if (lexer->last == 'm') {
            lexer_back(lexer);
            if (!lexer_multiplication(lexer, pair)) {
                continue;
            }
            return pair;
        }

        /* If there is a 'd', look for do/don't instruction. */
        else if (lexer->last == 'd') {

            if (lexer_next(lexer) != 'o') {
                continue;
            }

            /* Can either be "()" or "n't()" here */

            lexer_next(lexer);

            /* DO! */
            if (lexer->last == '(') {
                if (lexer_next(lexer) == ')') {
                    lexer->apply = true;
                    continue;
                }
            }

            /* DON'T! */
            else if (lexer->last == 'n') {
                if (lexer_next(lexer) != '\'') {
                    continue;
                }
                if (lexer_next(lexer) != 't') {
                    continue;
                }
                if (lexer_next(lexer) != '(') {
                    continue;
                }
                if (lexer_next(lexer) != ')') {
                    continue;
                }
                lexer->apply = false;
                continue;
            }
        }
    }

    return NULL;
}
