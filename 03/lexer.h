#ifndef _LEXER_H_
#define _LEXER_H_

#include <stdio.h>

typedef struct {
    FILE *stream;
    char last;
} lexer_t;

typedef struct {
    int a;
    int b;
} mulpair_t;

void lexer_create(lexer_t *lexer, FILE *stream);
mulpair_t *lexer_pair(lexer_t *lexer, mulpair_t *pair);

#endif // _LEXER_H_
