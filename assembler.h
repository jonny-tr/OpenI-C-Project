#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/* libraries */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* macros */
#define allocation_failure \
            fprintf(stderr, "Memory allocation failed\n");\
            exit(EXIT_FAILURE);

#define safe_free(p) if ((p) != NULL) { free(p); (p) = NULL; }

/* structures */
typedef struct {
    char *name;
    char *content;
} Macro;

/* functions */
int pre_assembler(int argc, char **argv);

#endif /* ASSEMBLER_H */
