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

/* pre_assembler functions */
int pre_assembler(int argc, char **argv);
int macro_table_builder(char *next_part, FILE *as_fd, Macro **macro_table,
                        int *macro_counter);
char *assembler_strdup(const char *s);
char *assembler_strcat(const char *s1, const char *s2);
int free_macro_table(Macro *macro_table, int macro_counter);
int is_macro(char *next_part, Macro **macro_table, int macro_counter);
int is_macro_name_valid(char *next_part, Macro **macro_table, int macro_counter);
int read_next_part(FILE *as_fd, char **next_part);
int macro_parser(FILE *as_fd, char *filename);


#endif /* ASSEMBLER_H */
