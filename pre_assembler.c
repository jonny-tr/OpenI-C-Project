#include "assembler.h"

/**
 * @brief skips the macro
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @param next_part the next part of the file
 * @param line_num the line number
 * @return line number, -1 if an error occurred
 */
int skip_macro(FILE *as_fd, char *filename, char *next_part, int *line_num) {
    int i, temp_flag; /* counter and flag*/

    do {
        if (strchr(next_part, '\n') == NULL) temp_flag = 1;
        if (read_next_part(as_fd, &next_part) != 0) return -1;
        for (i = 0; i < strlen(next_part); i++) {
            if (next_part[i] == '\n') {
                temp_flag = 0;
                (*line_num)++;
            }
        }
    } while (strcmp(next_part, "endmacr") != 0 && !feof(as_fd));

    if (!feof(as_fd)) {
        if (temp_flag == 1) {
            fprintf(stdout, "Error: line %d in %s.\n       "
                            "'endmacr' should be declared in a separate line.\n",
                    *line_num, filename);
        }
        if (read_next_part(as_fd, &next_part) != 0) return -1; /* spaces */
        if (strchr(next_part, '\n') == NULL && !feof(as_fd)) {
            fprintf(stdout, "Error: line %d in %s.\n       "
                            "Extra characters after 'endmacr'.\n",
                    *line_num, filename);
        } else {
            if (read_next_part(as_fd, &next_part) == -1) return 1;
            for (i = 0; i < strlen(next_part); i++)
                if (next_part[i] == '\n') (*line_num)++;
        }
    }

    /* file finished without endmacr */
    if (feof(as_fd)) {
        fprintf(stdout, "Error: %s finished without 'endmacr'.\n",
                filename);
    }
    return *line_num;
}

/**
 * @brief builds the macro table
 * @param next_part the next part of the file
 * @param as_fd the file pointer
 * @param macro_table_head the table of macros
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int macro_table_builder(char *next_part, FILE *as_fd,
                        macro_ptr *macro_table_head, int *line_num,
                        char *filename) {
    str_node_ptr macro_content_head = NULL, macro_content_tail = NULL,
                new_node = NULL; /* linked list of strings */
    macro_ptr new_macro; /* new macro */
    int i, error_flag = 0; /* counter and flags */
    char *buffer = NULL; /* buffer */

    if (read_next_part(as_fd, &next_part) != 0) {
        for (i = 0; i < strlen(next_part); i++)
            if (next_part[i] == '\n') (*line_num)++;
        return 1;
    }

    /* Create new macro node */
    new_macro = (macro_ptr)calloc(1, sizeof(macro_t));
    if (new_macro == NULL) {
        safe_free(next_part)
        free_macro_table(*macro_table_head);
        fclose(as_fd);
        allocation_failure
    }

    /* check if macro name is valid */
    switch (is_macro_name_valid(next_part, *macro_table_head)) {
        case 1:
            fprintf(stdout, "Error: line %d in %s.\n       "
                            "Macro name '%s' cannot be a saved word.\n",
                    *line_num, filename, next_part);
            error_flag = 1;
            if (skip_macro(as_fd, filename, next_part, line_num) == -1)
                return -1;
            break;
        case 2:
            fprintf(stdout, "Error: line %d in %s.\n       "
                            "Macro name '%s' already exists.\n",
                    *line_num, filename, next_part);
            error_flag = 1;
            if (skip_macro(as_fd, filename, next_part, line_num) == -1)
                return -1;
            break;
        case 0: /* valid name */
            if (as_strdup(&new_macro->name, next_part) != 0) {
                safe_free(next_part)
                free_macro_table(*macro_table_head);
                fclose(as_fd);
                allocation_failure
            }
            if (read_next_part(as_fd, &next_part) != 0) error_flag = 1;
            if (as_strdup(&buffer, next_part) != 0) {
                safe_free(next_part)
                free_macro_table(*macro_table_head);
                fclose(as_fd);
                allocation_failure
            }
            if (strchr(next_part, '\n') == NULL) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Extra characters after macro name.\n",
                        *line_num, filename);
                error_flag = 1;
            } else
                for (i = 0; i < strlen(next_part); i++)
                    if (next_part[i] == '\n') (*line_num)++;
            break;
        default:
            break;
    }

    /* run until macro is finished */
    while (!feof(as_fd)) {
        if (as_strdup(&buffer, next_part) != 0) {
            safe_free(next_part)
            free_macro_table(*macro_table_head);
            fclose(as_fd);
            allocation_failure
        }
        if (read_next_part(as_fd, &next_part) != 0) {
            error_flag = 1;
            break;
        }
        for (i = 0; i < strlen(next_part); i++) {
            if (next_part[i] == '\n') {
                (*line_num)++;
            }
        }

        if (strcmp(next_part, "endmacr") == 0) {
            if (strchr(buffer, '\n') == NULL) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "'endmacr' should be declared in a separate "
                                "line.\n", *line_num, filename);
                error_flag = 1;
            }
            if (read_next_part(as_fd, &next_part) != 0) {
                error_flag = 1;
                break;
            }
            if (strchr(next_part, '\n') == NULL
                    && !feof(as_fd)) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Extra characters after 'endmacr'.\n",
                                *line_num, filename);
                error_flag = 1;
            } else for (i = 0; i < strlen(next_part); i++)
                    if (next_part[i] == '\n') (*line_num)++;
            break;
        }

        /* add next_part to the macro content linked list */
        new_node = (str_node_ptr)calloc(1, sizeof(str_node_ptr));
        if (new_node == NULL
                || (as_strdup(&new_node->str, next_part) != 0)
                || (new_node->str == NULL)) {
            safe_free(next_part)
            free_macro_table(*macro_table_head);
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
    new_macro->next = *macro_table_head;
    *macro_table_head = new_macro;

    safe_free(buffer)

    if (error_flag) return -1;

    return 0;
}

/**
 * @brief frees the macro table
 * @param macro_table_head the table of macros
 * @return 0 if the function ran successfully, 1 if macro_table_head is NULL
 */
int free_macro_table(macro_ptr macro_table_head) {
    macro_t *current, *next;
    str_node_ptr current_content, *next_content;

    if ((current = macro_table_head) == NULL) return 1;

    while (current != NULL) {
        next = current->next;
        safe_free(current->name)
        current_content = current->content_head;
        while (current_content != NULL) {
            next_content = &current_content->next;
            safe_free(current_content->str)
            safe_free(current_content)
            current_content = *next_content;
        }
        safe_free(current)
        current = next;
    }

    return 0;
}

/**
 * @brief checks if the next part is a macro
 * @param next_part the next part of the file
 * @param macro_table_head the table of macros
 * @return a pointer to the macro, NULL if it is not a macro
 */
macro_ptr is_macro(char *next_part, macro_ptr macro_table_head) {
    macro_ptr current = macro_table_head;

    while (current != NULL) {
        if (strcmp(next_part, current->name) == 0) {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

/**
 * @brief checks if the macro name is valid
 * @param name the name of the macro
 * @param macro_table_head the table of macros
 * @return 0 if the name is valid, 1 if it is not, 2 if it is already taken
 */
int is_macro_name_valid(char *name, macro_ptr macro_table_head) {
    macro_t *current = macro_table_head;

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
 * @brief reads the next part of the file
 * @param fd the file pointer
 * @param next_part pointer to writing the next part
 * @return 0 if the function ran successfully, 1 if an error occurred,
 *         -1 if the file ended
 */
int read_next_part(FILE *fd, char **next_part) {
    int c, is_space; /* character, flag */
    char *temp = NULL; /* temporary pointer */
    size_t buffer = 0; /* buffer */

    if (next_part == NULL || *next_part == NULL) return 1;

    c = fgetc(fd);
    if (c == EOF) return 1;

    is_space = isspace(c) ? 1 : 0; /* normalize the result of is_space */

    do {
        if (buffer % 19 == 0) {
            temp = (char *)realloc(*next_part, buffer + 21);
            if (!temp) {
                safe_free(*next_part)
                fclose(fd);
                allocation_failure
            }
            *next_part = temp;
        }
        (*next_part)[buffer++] = (char)c;

        c = fgetc(fd);
        if (c == EOF) break;
    } while ((isspace(c) ? 1 : 0) == is_space);
    (*next_part)[buffer] = '\0';

    if (c != EOF) ungetc(c, fd);

    if (feof(fd)) return -1;

    return 0;
}

/**
 * @brief parses the macros in the file
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @param macro_table_head the start of macros table
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int macro_parser(FILE *as_fd, char *filename, macro_ptr *macro_table_head) {
    char *next_part = NULL, *content_buffer = NULL, *macro_buffer = NULL;
    /* strings */
    int i, line_num = 1, error_flag = 0; /* counters */
    unsigned long len; /* position counter */
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

    next_part = (char *)calloc(20, sizeof(char)); /* initial allocation */
    if (next_part == NULL) {
        fclose(as_fd);
        fclose(am_fd);
        allocation_failure
    }

    while (!feof(as_fd)) {
        if (as_strdup(&macro_buffer, next_part) != 0) {
            safe_free(next_part)
            fclose(am_fd);
            fclose(as_fd);
            allocation_failure
        }

        if (read_next_part(as_fd, &next_part) != 0) {
            break;
        }
        for (i = 0; i < strlen(next_part); i++)
            if (next_part[i] == '\n') line_num++;

        /* save macros in the macros table */
        if (strcmp(next_part, "macr") == 0) {
            if (strchr(macro_buffer, '\n') == NULL) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Macro should be declared in a separate line."
                                "\n", line_num, filename);
                error_flag = 1;
            }
            if (read_next_part(as_fd, &next_part) != 0
                        || (error_flag = macro_table_builder(next_part, as_fd,
                               macro_table_head, &line_num,
                               filename) != 0)) {
                for (i = 0; i < strlen(next_part); i++)
                    if (next_part[i] == '\n') line_num++;
            }
            continue;
        } else {
            macro_index = is_macro(next_part, *macro_table_head);
            if (macro_index != NULL && strchr(macro_buffer, '\n') == NULL) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Macro should be used in a separate line."
                                "\n", line_num, filename);
                error_flag = 1;
            } else if (macro_index != NULL) {
                if (read_next_part(as_fd, &next_part) == 0) {
                    for (i = 0; i < strlen(next_part); i++)
                        if (next_part[i] == '\n') line_num++;
                }
                if (strchr(next_part, '\n') == NULL) {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Macro should be used in a separate line."
                                    "\n", line_num, filename);
                    error_flag = 1;
                }
                content_node = macro_index->content_head;
                while (content_node != NULL && content_node->str != NULL) {
                    content_buffer = as_strcat(content_buffer,
                                               content_node->str);
                    content_node = content_node->next;
                }

                len = strlen(content_buffer);
                while (len > 0 && content_buffer != NULL &&
                        isspace(content_buffer[len - 1])) {
                    content_buffer[--len] = '\0';
                }

                fwrite(content_buffer, strlen(content_buffer), 1, am_fd);
                safe_free(content_buffer)
                macro_index = NULL;
            } else {
                fwrite(next_part, strlen(next_part), 1, am_fd);
            }
        }
    }

    /* free memory */
    safe_free(macro_buffer)
    safe_free(next_part)
    fclose(am_fd);

    if (error_flag) return -1;

    return 0;
}


/**
 * @brief pre-assembles the file
 * @param fd the file name
 * @param macro_table_head the head of macros table
 * @return 0 if the function ran successfully, -1 if an error occurred
 */
int pre_assembler(char **fd, macro_ptr macro_table_head) {
    FILE *as_fd; /* file pointer */
    char *in_fd = as_strcat(*fd, ".as"); /* file name */

    if (in_fd == NULL) {
        allocation_failure
    }

    as_fd = fopen(in_fd, "r");
    if (as_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", in_fd);
        return 1;
    }

    switch (macro_parser(as_fd, in_fd, &macro_table_head)) {
        default:
            safe_free(in_fd)
            fclose(as_fd);
        case 0:
            return 0;
        case -1:
            return -1;
    }
}
