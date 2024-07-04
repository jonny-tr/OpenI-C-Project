#include "assembler.h"

/**
 * @brief duplicates a string
 * @param s a string
 * @return new string
 */
char *assembler_strdup(const char *s) {
    char *new_str = malloc(strlen(s) + 1);
    if (new_str == NULL) return NULL;
    strcpy(new_str, s);
    strcat(new_str, "\0");

    return new_str;
}

/**
 * @brief frees the macro table
 * @param macro_table the table of macros
 * @param macro_counter the counter of macros
 * @return 0
 */
int free_macro_table(Macro *macro_table, int macro_counter) {
    int i; /* counter */

    for (i = 0; i < macro_counter; ++i) {
        safe_free(macro_table[i].name)
        safe_free(macro_table[i].content)
    }
    safe_free(macro_table)

    return 0;
}

/**
 * @brief checks if  next part is a macro
 * @param next_part the part read from the file
 * @param macro_table the table of macros
 * @param macro_counter the counter of macros
 * @return the index of the macro if it is a macro, 0 if it is not
 */
int is_macro(char *next_part, Macro *macro_table, int macro_counter) {
    int i; /* counter */

    for (i = 0; i < macro_counter; ++i) {
        if (strcmp(next_part, macro_table[i].name) == 0) {
            return i;
        }
    }

    return 0;
}

/**
 * @brief checks if the macro name is valid
 * @param name the name of the macro
 * @return 0 if the name is valid, 1 if it is not
 */
int is_macro_name_valid(char *name, Macro *macro_table, int macro_counter) {
    char *invalid[] = {".data", ".string", ".entry", ".extern",
                      "mov", "cmp", "add", "sub", "lea",
                      "clr", "not", "inc", "dec", "jmp",
                      "bne", "red", "prn", "jsr", "rts",
                      "stop"}; /* invalid names */
    unsigned int i; /* counter */

    /* check if macro name is a saved word */
    for (i = 0; i < 20; ++i) {
        if (strcmp(name, invalid[i]) == 0) {
            fprintf(stderr, "Error: Macro name cannot "
                            "be a special word - %s\n", name);
            return 1;
        }
    }

    /* check if macro name is already taken */
    for (i = 0; i < macro_counter; ++i) {
        if (strcmp(name, macro_table[i].name) == 0) {
            fprintf(stderr, "Error: Macro name already exists - %s\n", name);
            return 1;
        }
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
    char *next_part = NULL, *macro_content = NULL, *temp = NULL; /* strings */
    int macro_counter = 0, i; /* macro counter */
    FILE *am_fd; /* file pointer */
    Macro *macro_table = NULL, *temp_macro; /* macro table */

    macro_table = (Macro *)calloc(20, sizeof(Macro));
    if (macro_table == NULL) {
        fclose(as_fd);
        allocation_failure
    }

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
        allocation_failure
    }

    while (!feof(as_fd)) {
        read_next_part(as_fd, &next_part);

        /* save macros in the macros table */
        if (strlen(next_part) >= 4
                && strncmp(next_part, "macr", 4) == 0) {
            read_next_part(as_fd, &next_part);

            /* realloc macro_table if there are too many macros */
            if (macro_counter % 19 == 0) {
                temp_macro = realloc(macro_table,
                                     sizeof(Macro) * (macro_counter + 20));
                if (temp_macro == NULL) {
                    safe_free(next_part)
                    free_macro_table(macro_table, macro_counter);
                    fclose(as_fd);
                    allocation_failure
                }
                macro_table = temp_macro;
            }

            /* check if macro name is valid */
            if (!is_macro_name_valid(next_part, macro_table, macro_counter)) {
                macro_table[macro_counter].name = assembler_strdup(next_part);
            } else {
                do {
                    read_next_part(as_fd, &next_part); /* skip macro */
                } while (strcmp(next_part, "endmacr") != 0);
                safe_free(next_part)
                continue;
            }

            /* run until macro is finished */
            while (!feof(as_fd)) {
                if (macro_content == NULL) {
                    macro_content = assembler_strdup(next_part);
                } else {
                    if (!(temp = realloc(macro_content,
                                         strlen(macro_content)
                                         + strlen(next_part) + 2))) {
                        safe_free(next_part)
                        safe_free(macro_content)
                        allocation_failure
                    }
                    macro_content = temp;
                    strcat(macro_content, next_part);
                }
                read_next_part(as_fd, &next_part);

                if (strcmp(next_part, "endmacr") == 0) {
                    read_next_part(as_fd, &next_part); /* skip spaces */
                    read_next_part(as_fd, &next_part);
                    break;
                }
            }

            macro_table[macro_counter].content =
                    assembler_strdup(macro_content);
            safe_free(macro_content)
            ++macro_counter;
        }

        for (i=0; i<macro_counter; i++) {
            fprintf(stderr, "%s\n", macro_table[i].name);
            fprintf(stderr, "%s\n", macro_table[i].content);
        }
        /* write next part to the new am file */
        if ((i = is_macro(next_part, macro_table, macro_counter))) {
            fwrite(macro_table[i].content, strlen(macro_table[i].content),
                   1, am_fd);
        } else {
            fwrite(next_part, strlen(next_part), 1, am_fd);
        }
    }

    /* free memory */
    free_macro_table(macro_table, macro_counter);
    safe_free(next_part)

    return 0;
}


/**
 * @brief pre-assembler function
 * @param argc the number of arguments
 * @param argv an array of arguments
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int pre_assembler(int argc, char **argv) {
    unsigned int i; /* counter */
    FILE *fd; /* file pointer */

    for (i = 1; i < argc; i++) {
        /* open the file */
        argv[i] = strcat(argv[i], ".as");
        fd = fopen(argv[i], "r");
        if (fd == NULL) {
            fprintf(stderr, "Error: Could not open file %s\n", argv[i]);
            return 1;
        }

        macro_parser(fd, argv[i]);
        /* close the file */
        fclose(fd);
    }

    return 0;
}
