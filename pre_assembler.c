#include "assembler.h"

/**
 * @brief duplicates a string
 * @param s a string
 * @return new string
 */
char *assembler_strdup(const char *s) {
    char *new_str = calloc(strlen(s) + 1, sizeof(char));

    if (new_str == NULL) return NULL;

    strcpy(new_str, s);
    strcat(new_str, "\0");

    return new_str;
}

/**
 * @brief concatenates two strings
 * @param s1 the first string
 * @param s2 the second string
 * @return new string
 */
char *assembler_strcat(const char *s1, const char *s2) {
    const char *safe_s1 = s1 ? s1 : "";
    const char *safe_s2 = s2 ? s2 : "";
    char *new_str = calloc(strlen(safe_s1) + strlen(safe_s2) + 1, sizeof(char));

    if (new_str == NULL) return NULL;
    strcpy(new_str, safe_s1);
    strcat(new_str, safe_s2);

    return new_str;
}

/**
 * @brief builds the macro table
 * @param next_part the next part of the file
 * @param as_fd the file pointer
 * @param macro_table_head the table of macros
 * @return 0
 */
int macro_table_builder(char *next_part, FILE *as_fd,
                        macro_ptr *macro_table_head, int *line_num,
                        char *filename) {
    str_node_ptr macro_content_head = NULL, macro_content_tail = NULL,
        new_node = NULL; /* linked list of strings */
    macro_ptr new_macro; /* new macro */

    if (read_next_part(as_fd, &next_part) != 0) {
        if (strchr(next_part, '\n') != NULL) line_num++;
        return 1;
    }

    /* Create new macro node */
    new_macro = (macro_ptr)calloc(1, sizeof(Macro));

    if (new_macro == NULL) {
        safe_free(next_part)
        free_macro_table(*macro_table_head);
        fclose(as_fd);
        allocation_failure
    }

    /* check if macro name is valid */
    if (!is_macro_name_valid(next_part, *macro_table_head)) {
        new_macro->name = assembler_strdup(next_part);
        read_next_part(as_fd, &next_part); /* skip spaces */
        if (strchr(next_part, '\n') == NULL) {
            fprintf(stdout, "Error: Extra characters after macro name.\n"
                            "Review line %d in %s\n", *line_num, filename);
            safe_free(next_part)
            free_macro_table(*macro_table_head);
            return 1;
        } else line_num++;
        read_next_part(as_fd, &next_part); /* read next part */
        if (strchr(next_part, '\n') != NULL) line_num++;
    } else {
        safe_free(new_macro)
        do {
            read_next_part(as_fd, &next_part); /* skip macro */
            if (strchr(next_part, '\n') != NULL) line_num++;
        } while (strcmp(next_part, "endmacr") != 0);
        safe_free(next_part)
        /* file finished without endmacr */
        if (feof(as_fd)) {
            fprintf(stdout, "Error: Unexpected end of file.\n");
        } else {
            read_next_part(as_fd, &next_part); /* skip spaces */
            if (strchr(next_part, '\n') != NULL) line_num++;
        }
        return 1;
    }

    /* run until macro is finished */
    while (!feof(as_fd)) {
        if (strcmp(next_part, "endmacr") == 0) {
            read_next_part(as_fd, &next_part); /* spaces */
            if (strchr(next_part, '\n') == NULL
                    && !feof(as_fd)) {
                fprintf(stdout, "Error: Extra characters after endmacr.\n");
                return 1;
            } else line_num++;
            break;
        }

        /* add next_part to the macro content linked list */
        new_node = (str_node_ptr)calloc(1, sizeof(str_node_ptr));
        if (new_node == NULL) {
            safe_free(next_part)
            free_macro_table(*macro_table_head);
            fclose(as_fd);
            allocation_failure
        }
        new_node->str = assembler_strdup(next_part);
        if (macro_content_tail == NULL) {
            macro_content_head = macro_content_tail = new_node;
        } else {
            macro_content_tail->next = new_node;
            macro_content_tail = new_node;
        }
        read_next_part(as_fd, &next_part);
        if (strchr(next_part, '\n') != NULL) line_num++;
    }

    new_macro->content_head = macro_content_head;
    new_macro->next = *macro_table_head;
    *macro_table_head = new_macro;

    return 0;
}

/**
 * @brief frees the macro table
 * @param macro_table_head the table of macros
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int free_macro_table(macro_ptr macro_table_head) {
    Macro *current, *next;
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
 * @return the index of the macro if it is a macro, -1 if it is not
 */
int is_macro(char *next_part, macro_ptr macro_table_head) {
    macro_ptr current = macro_table_head;
    int index = 0;

    while (current != NULL) {
        if (strcmp(next_part, current->name) == 0) {
            return index;
        }
        current = current->next;
        index++;
    }

    return -1;
}

/**
 * @brief checks if the macro name is valid
 * @param name the name of the macro
 * @param macro_table_head the table of macros
 * @return 0 if the name is valid, 1 if it is not
 */
int is_macro_name_valid(char *name, macro_ptr macro_table_head) {
    char *invalid[] = {".data", ".string", ".entry", ".extern",
                       "mov", "cmp", "add", "sub", "lea",
                       "clr", "not", "inc", "dec", "jmp",
                       "bne", "red", "prn", "jsr", "rts",
                       "stop"}; /* invalid names */
    unsigned int i; /* counter */
    Macro *current = macro_table_head;

    /* check if macro name is a saved word */
    for (i = 0; i < 20; ++i) {
        if (strcmp(name, invalid[i]) == 0) {
            fprintf(stdout, "Error: Macro name cannot "
                            "be a reserved word - %s.\n", name);
            return 1;
        }
    }

    /* check if macro name is already taken */
    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            fprintf(stdout, "Error: Macro name already exists - %s.\n", name);
            return 1;
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief reads the next part of the file
 * @param fd the file pointer
 * @param next_part pointer to writing the next part
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int read_next_part(FILE *fd, char **next_part) {
    int c, is_space; /* character, flag */
    char *temp = NULL; /* temporary pointer */
    size_t buffer = 0; /* buffer */

    if (next_part == NULL || *next_part == NULL) return 1;

    c = fgetc(fd);
    if (c == EOF) return 1;

    /* normalize the result of is_space */
    is_space = isspace(c) ? 1 : 0;

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

    return 0;
}

/**
 * @brief parses the macros in the file
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int macro_parser(FILE *as_fd, char *filename) {
    char *next_part = NULL, *content_buffer = NULL, *macro_buffer = NULL;
    /* strings */
    int macro_index, i, line_num = 1;
    unsigned long len; /* counters */
    FILE *am_fd; /* file pointer */
    Macro *macro_table_head = NULL, *current = NULL; /* macro table */
    str_node_ptr content_node; /* content node */

    /* create a new file with the .am suffix */
    filename[strlen(filename) - 1] = 'm';
    am_fd = fopen(filename, "w");
    if (am_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", filename);
        return 1;
    }
    filename[strlen(filename) - 1] = 's';
    /* initial allocation */
    if (!(next_part = (char *)calloc(20, sizeof(char)))) {
        fclose(as_fd);
        fclose(am_fd);
        allocation_failure
    }

    while (!feof(as_fd)) {
        macro_buffer = assembler_strdup(next_part);
        if (read_next_part(as_fd, &next_part) != 0) {
            break;
        }
        if (strchr(next_part, '\n') != NULL) line_num++;

        /* save macros in the macros table */
        if (strcmp(next_part, "macr") == 0) {
            if (strchr(macro_buffer, '\n') == NULL) {
                fprintf(stdout, "Error: Macro should be declared "
                                "in a separate line.\n"
                                "Review line %d in %s.\n", line_num, filename);
                return 1;
            }
            else if (read_next_part(as_fd, &next_part) != 0
                    || macro_table_builder(next_part, as_fd,
                                       &macro_table_head, &line_num,
                                       filename) != 0) {
                if (strchr(next_part, '\n') != NULL) line_num++;
                return 1;
            }
        } else {
            macro_index = is_macro(next_part, macro_table_head);
            if (macro_index > -1) {
                current = macro_table_head;
                for (i = 0; i < macro_index; i++) {
                    current = current->next;
                }

                content_node = current->content_head;
                while (content_node != NULL && content_node->str != NULL) {
                    content_buffer = assembler_strcat(content_buffer,
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
            } else {
                fwrite(next_part, strlen(next_part), 1, am_fd);
            }
        }
    }

    /* free memory */
    free_macro_table(macro_table_head);
    safe_free(macro_buffer)
    safe_free(next_part)
    fclose(am_fd);

    return 0;
}


/**
 * @brief pre-assembles the file
 * @param in_fd the file name
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int pre_assembler(char **in_fd) {
    FILE *as_fd; /* file pointer */

    *in_fd = strcat(*in_fd, ".as");
    as_fd = fopen(*in_fd, "r");
    if (as_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", *in_fd);
        return 1;
    }

    switch (macro_parser(as_fd, *in_fd)) {
        default:
            fclose(as_fd);
        case 0:
            return 0;
        case 1:
            return 1;
    }
}
