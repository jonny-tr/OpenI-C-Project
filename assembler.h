#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/* libraries */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* macros */
#define allocation_failure \
            fprintf(stdout, "Memory allocation failed\n");\
            exit(EXIT_FAILURE);

#define safe_free(p) if ((p) != NULL) { free(p); (p) = NULL; }

/* structures */
typedef struct StringNode {
    char *str;
    struct StringNode *next;
} StringNode;

typedef StringNode *str_node_ptr;


typedef struct Macro {
    char *name;
    StringNode *content_head;
    struct Macro *next;
} Macro;

typedef Macro *macro_ptr;

typedef Macro *macro_ptr;

/* pre_assembler functions */
int pre_assembler(char **in_fd);
int macro_table_builder(char *next_part, FILE *as_fd,
                        macro_ptr *macro_table_head, int *line_num,
                        char *filename);
char *assembler_strdup(const char *s);
char *assembler_strcat(const char *s1, const char *s2);
int free_macro_table(macro_ptr macro_table_head);
int is_macro(char *next_part, macro_ptr macro_table_head);
int is_macro_name_valid(char *name, macro_ptr macro_table_head);
int read_next_part(FILE *as_fd, char **next_part);
int macro_parser(FILE *as_fd, char *filename);


#endif /* ASSEMBLER_H */
