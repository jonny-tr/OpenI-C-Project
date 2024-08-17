#include "assembler.h"

/**
 * @brief skips the macro in the .as file
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @param line_num the line number
 * @return 0 on success, -1 if an error occurred
 */
int skip_macro(FILE *as_fd, char *filename, int *line_num) {
    int word_counter, error_flag, endmacr = 0; /* counter and flag*/
    char word[LINE_SIZE] = {0}, line[LINE_SIZE] = {0}, *word_ptr; /* string */

    while (read_next_line(as_fd, line)) {
        (*line_num)++;
        word_counter = 0;
        word_ptr = line;
        while (get_next_word(word, &word_ptr) != -1) {
            word_counter++;
            if (strcmp(word, "endmacr") == 0) {
                endmacr = 1;
                if (word_counter > 1) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "'endmacr' should be declared in a "
                                    "separate line.\n", *line_num, filename);
                    error_flag = 1;
                } else if (get_next_word(word, &word_ptr) != -1) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Extra characters after 'endmacr'.\n",
                            *line_num, filename);
                    error_flag = 1;
                }
            }
        }

        if (endmacr) break;

        /* file finished without endmacr */
        if (feof(as_fd)) {
            fprintf(stdout, "Error: %s finished without 'endmacr'.\n",
                    filename);
            error_flag = 1;
        }
    }

    if (error_flag) return -1;

    return 0;
}

/**
 * @brief builds the macro table
 * @param next_part the next part of the file
 * @param as_fd the file pointer
 * @param macro_head the table of macros
 * @param line_num the line number
 * @param filename the name of the file
 * @param macro_name the name of the macro
 * @return 0 on success, -1 if an error occurred
 */
int macro_table_builder(FILE *as_fd, macro_ptr *macro_head, int *line_num,
                        char *filename, char *macro_name) {
    char buffer[LINE_SIZE] = {0}, word[LINE_SIZE] = {0},
            line[LINE_SIZE] = {0}, *line_ptr; /* strings */
    int word_counter, eol_flag, endmacr_flag, error_flag = 0;
    /* counter and flags */
    macro_ptr new_macro; /* new macro */
    str_node_ptr macro_content_head = NULL, macro_content_tail = NULL,
            new_node = NULL; /* linked list of strings */

    /* create a new macro node */
    new_macro = (macro_ptr) calloc(1, sizeof(macro_t));
    if (new_macro == NULL) {
        free_macro_table(*macro_head);
        fclose(as_fd);
        allocation_failure
    }

    if (as_strdup(&new_macro->name, macro_name) != 0) {
        free_macro_table(*macro_head);
        fclose(as_fd);
        allocation_failure
    }

    /* run until macro is finished */
    while (read_next_line(as_fd, line) != -1) {
        (*line_num)++;
        line_ptr = line;
        endmacr_flag = 0;

        while (isspace((unsigned char) line_ptr[0])) line_ptr++;
        if (line_ptr[0] == ';') continue; /* comment */
        if (get_next_word(word, &line_ptr) == -1) continue; /* empty */

        word_counter = 0;
        do {
            word_counter += 1;
            if (word_counter > 1 && strcmp(word, "endmacr") == 0) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "'endmacr' should be declared in a separate "
                                "line.\n", *line_num, filename);
                error_flag = 1;
            } else if (word_counter == 1 && strcmp(word, "endmacr") == 0) {
                if ((eol_flag
                             = get_next_word(buffer, &line_ptr)) != -1) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Extra characters after 'endmacr'.\n",
                            *line_num, filename);
                    error_flag = 1;
                } else if (eol_flag == -1) {
                    endmacr_flag = 1;
                    break;
                }
            }
        } while (get_next_word(word, &line_ptr) != -1);

        if (endmacr_flag) break;

        /* add next_part to the macro content linked list */
        new_node = (str_node_ptr) calloc(1, sizeof(str_node_ptr));
        if (new_node == NULL
            || (as_strdup(&new_node->str, line) != 0)
            || (new_node->str == NULL)) {
            free_macro_table(*macro_head);
            fclose(as_fd);
            allocation_failure
        }

        if (macro_content_tail == NULL) {
            macro_content_head = macro_content_tail = new_node;
        } else {
            macro_content_tail->next = new_node;
            macro_content_tail = new_node;
        }
    }

    new_macro->content_head = macro_content_head;
    new_macro->next = *macro_head;
    *macro_head = new_macro;

    if (error_flag) return -1;

    return 0;
}

/**
 * @brief checks if the next part is a macro
 * @param word the next part of the file
 * @param macro_head the table of macros
 * @return a pointer to the macro, NULL if it is not a macro
 */
macro_ptr is_macro(char *word, macro_ptr macro_head) {
    macro_ptr current = macro_head;

    while (current != NULL) {
        if (strcmp(word, current->name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

/**
 * @brief checks if the macro name is valid
 * @param name the name of the macro
 * @param macro_head the table of macros
 * @return 0 if the name is valid, 1 if it is not, 2 if it is already taken
 */
int is_macro_name_valid(char *name, macro_ptr macro_head) {
    macro_t *current = macro_head;

    /* check if macro name is a saved word */
    if (is_valid_command(name) != -1) return 1;

    /* check if macro name is already taken */
    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            return 2;
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief parses the macros in the file
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @param macro_head the start of macros table
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int macro_parser(FILE *as_fd, char *filename, macro_ptr *macro_head) {
    char line[LINE_SIZE] = {0}, *content_buffer = NULL, *macro_buffer = NULL,
            word[LINE_SIZE] = {0}, *word_ptr = NULL, buffer[LINE_SIZE] = {0};
    /* strings */
    int line_num = 0, word_counter, macro_flag = 0, error_flag = 0;
    /* counters and flags */
    FILE *am_fd; /* file pointer */
    macro_ptr macro_index = NULL; /* macro to spread */
    str_node_ptr content_node; /* content node */

    /* create a new file with the .am suffix */
    filename[strlen(filename) - 1] = 'm';
    am_fd = fopen(filename, "w");
    if (am_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", filename);
        return -1;
    }
    filename[strlen(filename) - 1] = 's';

    while (read_next_line(as_fd, line) != -1) {
        line_num += 1;
        word_ptr = line;

        if (get_next_word(word, &word_ptr) == -1) continue; /* empty */

        /* loop words in line */
        word_counter = 0;
        do {
            word_counter += 1;
            if (strcmp(word, "macr") == 0) {
                if (word_counter > 1) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Macro should be declared in a separate "
                                    "line.\n", line_num, filename);
                    error_flag = 1;
                } else {
                    macro_flag = 1;
                    break;
                }
            }
            macro_index = is_macro(word, *macro_head);
            if (macro_index != NULL && word_counter > 1) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Macro should be used in a separate line."
                                "\n", line_num, filename);
                error_flag = 1;
            } else if (macro_index != NULL) {
                content_node = macro_index->content_head;
                while (content_node != NULL && content_node->str != NULL) {
                    content_buffer = as_strcat(content_buffer,
                                               content_node->str);
                    content_node = content_node->next;
                }
                fprintf(am_fd, "%s", content_buffer);
                safe_free(content_buffer)
            }
        } while (get_next_word(word, &word_ptr) != -1);

        if (macro_flag) {
            if (get_next_word(word, &word_ptr) == -1) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Macro name is missing.\n", line_num, filename);
                error_flag = 1;
            }

            if (get_next_word(buffer, &word_ptr) != -1) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Extra characters after macro name.\n",
                        line_num, filename);
                error_flag = 1;
            }

            /* check if macro name is valid */
            switch (is_macro_name_valid(word, *macro_head)) {
                case 1:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Macro name '%s' cannot be a saved word.\n",
                            line_num, filename, word);
                    error_flag = 1;
                    if (skip_macro(as_fd, filename, &line_num) == -1)
                        error_flag = 1;
                    break;
                case 2:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Macro name '%s' already exists.\n",
                            line_num, filename, word);
                    error_flag = 1;
                    if (skip_macro(as_fd, filename, &line_num) == -1)
                        error_flag = 1;
                    break;
                case 0: /* valid name */
                    if (macro_table_builder(as_fd, macro_head, &line_num,
                                             filename, word) != 0) {
                        error_flag = 1;
                    }
                    break;
                default:
                    break;
            }
            macro_flag = 0;
        } else if (macro_index == NULL) {
            fprintf(am_fd, "%s", line);
        } else {
            macro_index = NULL;
        }
    }

    /* free memory */
    safe_free(macro_buffer)

    if (error_flag) {
        filename[strlen(filename) - 1] = 'm';
        if (remove(filename) != 0) {
            fprintf(stdout, "Error: Could not delete %s.\n", filename);
        }
        return -1;
    }

    fclose(am_fd);

    return 0;
}


/**
 * @brief pre-assembles the file
 * @param fd the file name
 * @param macro_head the head of macros table
 * @return 0 if the function ran successfully, -1 if an error occurred
 */
int pre_assembler(char **fd, macro_ptr macro_head) {
    FILE *as_fd; /* file pointer */
    char *in_fd = as_strcat(*fd, ".as"); /* file name */

    if (in_fd == NULL) {
        allocation_failure
    }

    as_fd = fopen(in_fd, "r");
    if (as_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", in_fd);
        return -1;
    }

    switch (macro_parser(as_fd, in_fd, &macro_head)) {
        default:
            safe_free(in_fd)
            fclose(as_fd);
        case 0:
            return 0;
        case -1:
            return -1;
    }
}
