#ifndef ASSEMBLER_H
#define ASSEMBLER_H

/*--------------------------------libraries----------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

/*----------------------------------macros-----------------------------------*/
#define LINE_SIZE 81

#define INVALID_INT INT_MIN

#define allocation_failure \
            fprintf(stdout, "Memory allocation failed.\n");\
            exit(EXIT_FAILURE);

#define next_word_check if (get_next_word(word, &word_ptr) \
                == -1) { \
            fprintf(stdout, "Error: Failed to read from %s.\n", filename); \
            error_flag = 1; \
            break; \
            }

#define safe_free(p) if ((p) != NULL) { free(p); (p) = NULL; }

/*---------------------------------enums-------------------------------------*/
enum word_type_e {
    ERROR = -1,
    LABEL = 0,
    DATA = 1,
    STRING = 2,
    ENTRY = 3,
    EXTERN = 4,
    COMMAND = 5,
    OPERAND = 6
};

/*--------------------------------structures---------------------------------*/
typedef struct string_t {
    char *str;
    struct string_t *next;
} string_t;
typedef string_t *str_node_ptr;

typedef struct macro_t {
    char *name;
    string_t *content_head;
    struct macro_t *next;
} macro_t;
typedef macro_t *macro_ptr;

typedef struct symbol_t {
    char *name; /* label */
    int counter; /* IC or NULL */
    char *type; /* data/external/entry */
    struct symbol_t *next;
} symbol_t;
typedef symbol_t *symbol_ptr;

typedef struct variable_t {
    int content: 15;
    int counter; /* DC */
    struct variable_t *next;
} variable_t;
typedef variable_t *variable_ptr;

typedef struct command_t {
    unsigned int are: 3; /* bits 02-00 */
    unsigned int dest_addr: 4; /* bits 06-03 */
    unsigned int src_addr: 4;  /* bits 10-07 */
    unsigned int opcode: 4;  /* bits 14-11 */
    unsigned int l: 3;
    struct command_t *next;
} command_t;
typedef command_t *command_ptr;

/*---------------------------------functions---------------------------------*/
/* text utils */
int as_strdup(char **dest, const char *s);
char *as_strcat(const char *s1, const char *s2);
int is_valid_command(char *command);
int read_next_line(FILE *fd, char *line);
int read_next_part(FILE *as_fd, char **next_part);
int get_next_word(char *word, char **word_ptr);
int get_word_type(char *word);
int command_to_num(command_ptr cmd);
int twos_complement(int num);
int comma_checker(char **word_ptr);
int get_ascii_value(char ch);

/* pre_assembler */
int pre_assembler(char **in_fd, macro_ptr macro_head);
int macro_table_builder(char *next_part, FILE *as_fd,
                        macro_ptr *macro_table_head, int *line_num,
                        char *filename);
macro_ptr is_macro(char *next_part, macro_ptr macro_head);
int is_macro_name_valid(char *name, macro_ptr macro_head);
int macro_parser(FILE *as_fd, char *filename, macro_ptr *macro_head);

/* phase_one */
int phase_one(FILE *am_fd, char *filename, int *ic, int *dc,
              symbol_ptr *symbol_head, variable_ptr *variable_head,
              command_ptr *command_head, macro_ptr *macro_head);
int is_valid_label(char *word, symbol_ptr symbol_head,
                   macro_ptr macro_head);
int is_valid_operand(char *word, macro_ptr macro_head);
int add_symbol(symbol_ptr *head, char *name, int counter, char *type);
int add_variable(variable_ptr *head, int content, int counter);
int init_command_word(command_ptr *head, command_ptr *ptr);
void set_command_opcode(command_t *field, int command);
void set_addressing_method(char *operand, command_ptr field, int src_dest);
int calc_l(command_ptr command, int cmnd);
void phase_one_update_counter(symbol_ptr symbol_head, int ic);
int get_data_int(char *word);


/* phase_two */
int build_ent(FILE *ent_fd, symbol_ptr symbol_head);
int build_ob(FILE *ob_fd, command_ptr command_head, variable_ptr variable_head,
             int ic, int dc);
int build_ext(FILE *ext_fd, symbol_ptr symbol_head);
int is_symbol(char *name, symbol_ptr symbols_head, command_ptr are,
              FILE **ext_fd, char *ext_file, int line_num);
int update_command_list(command_ptr command_list, char *word, char **word_ptr,
                        char *filename, symbol_ptr symbol_head,
                        FILE **ext_fd, char *ext_file, int line_num);
int update_entry(symbol_ptr symbol_head, char *word, char *filename,
                 int line_num);
int phase_two(FILE *am_fd, char *filename, symbol_ptr symbol_head,
              variable_ptr variable_head, command_ptr command_head,
              int expected_ic, int dc);

/* cleanup */
int free_macro_table(macro_ptr macro_head);
int free_symbols_table(symbol_ptr symbol_head);
int free_command_list(command_ptr command_head);
int free_variable_list(variable_ptr variable_head);
int free_all(macro_ptr macro_table_head, symbol_ptr symbols_list_head,
             variable_ptr var_list_head, command_ptr cmd_list_head);

#endif /* ASSEMBLER_H */
