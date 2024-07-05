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
                        macro_ptr *macro_table_head) {
    char *macro_content = NULL; /* string */
    macro_ptr new_macro; /* new macro */
    int len; /* length */

    read_next_part(as_fd, &next_part);

    /* Create new macro node */
    new_macro = malloc(sizeof(Macro));
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
            fprintf(stderr, "Error: Extra characters after macro name\n");
            safe_free(next_part)
            free_macro_table(*macro_table_head);
            fclose(as_fd);
            return 1;
        }
        read_next_part(as_fd, &next_part); /* read next part */
    } else {
        free(new_macro);
        do {
            read_next_part(as_fd, &next_part); /* skip macro */
        } while (strcmp(next_part, "endmacr") != 0);
        safe_free(next_part)
        return 1;
    }

    /* run until macro is finished */
    while (!feof(as_fd)) {
        if (strcmp(next_part, "endmacr") == 0) {
            /* remove previous spaces */
            len = (int)strlen(macro_content);
            while (len > 0 && isspace((unsigned char)macro_content[len - 1])) {
                macro_content[--len] = '\0';
            }
            read_next_part(as_fd, &next_part); /* spaces */
            if (strchr(next_part, '\n') == NULL) {
                fprintf(stderr, "Error: Extra characters after endmacr\n");
                safe_free(macro_content)
                return 1;
            }
            break;
        }
        macro_content = assembler_strcat(macro_content, next_part);
        read_next_part(as_fd, &next_part);
    }

    new_macro->content = assembler_strdup(macro_content);
    new_macro->next = *macro_table_head;
    *macro_table_head = new_macro;
    safe_free(macro_content)
    safe_free(next_part)

    return 0;
}

/**
 * @brief frees the macro table
 * @param macro_table_head the table of macros
 * @return 0
 */
int free_macro_table(macro_ptr macro_table_head) {
    Macro *current, *next;

    current = macro_table_head;
    while (current != NULL) {
        next = current->next;
        safe_free(current->name)
        safe_free(current->content)
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
            fprintf(stderr, "Error: Macro name cannot "
                            "be a special word - %s\n", name);
            return 1;
        }
    }

    /* check if macro name is already taken */
    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            fprintf(stderr, "Error: Macro name already exists - %s\n", name);
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

    if (next_part == NULL || *next_part == NULL)
        return 1;

    c = fgetc(fd);

    if (c == EOF)
        return 1;

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

    if (c != EOF) {
        ungetc(c, fd);
    }

    return 0;
}

/**
 * @brief parses the macros in the file
 * @param as_fd the file pointer
 * @param filename the name of the file
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int macro_parser(FILE *as_fd, char *filename) {
    char *next_part = NULL; /* strings */
    int macro_index, i; /* macro counter */
    FILE *am_fd; /* file pointer */
    Macro *macro_table_head = NULL; /* macro table */

    /* create a new file with the .am suffix */
    filename[strlen(filename) - 1] = 'm';
    am_fd = fopen(filename, "w");
    if (am_fd == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", filename);
        return 1;
    }

    /* initial allocation */
    if (!(next_part = (char *)calloc(20, sizeof(char)))) {
        fclose(as_fd);
        fclose(am_fd);
        allocation_failure
    }

    while (!feof(as_fd)) {
        if (read_next_part(as_fd, &next_part) != 0) {
            break;
        }

        /* save macros in the macros table */
        if (strlen(next_part) >= 4 && strncmp(next_part, "macr", 4) == 0) {
            read_next_part(as_fd, &next_part);
            if (macro_table_builder(next_part, as_fd,
                                    &macro_table_head) == 1) {
                fclose(am_fd);
                return 1;
            }
        } else {
            macro_index = is_macro(next_part, macro_table_head);
            if (macro_index > -1) {
                Macro *current = macro_table_head;
                for (i = 0; i < macro_index; i++) {
                    current = current->next;
                }
                fwrite(current->content, strlen(current->content), 1, am_fd);
            } else {
                fwrite(next_part, strlen(next_part), 1, am_fd);
            }
        }
    }

    /* free memory */
    free_macro_table(macro_table_head);
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
        fprintf(stderr, "Error: Could not open file %s\n", *in_fd);
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
