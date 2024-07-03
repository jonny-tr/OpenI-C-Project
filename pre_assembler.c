#include "assembler.h"

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
 * @return 1 if the name is valid, 0 if it is not
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
            return 0;
        }
    }

    /* check if macro name is already taken */
    for (i = 0; i < macro_counter; ++i) {
        if (strcmp(name, macro_table[i].name) == 0) {
            fprintf(stderr, "Error: Macro name already exists - %s\n", name);
            return 0;
        }
    }

    return 1;
}

/**
 * @brief reads the next part of the file
 * @param fd the file pointer
 * @param next_part pointer to writing the next part
 * @return 0 if the function ran successfully, 1 if an error occurred
 */
int read_next_part(FILE *fd, char *next_part) {
    char c = 0; /* character */
    char *temp = NULL; /* temporary pointer for realloc */
    size_t buffer = 0; /* buffer */

    /* read file until locating a space */
    while (!isspace(c)) {
        if (buffer >= 19 &&
            !(temp = (char *)realloc(next_part,
                                          sizeof(char) * (buffer + 2)))) {
            safe_free(next_part)
            allocation_failure
        }
        next_part = temp;
        fread(&c, sizeof(char), 1, fd);
        next_part[buffer] = (char) c;
        ++buffer;
    }
    next_part[buffer] = c;

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
    int macro_counter = 0, i = 0; /* macro counter */
    FILE *am_fd; /* file pointer */
    Macro *macro_table = NULL; /* macro table */

    macro_table = (Macro *)calloc(100, sizeof(Macro));
    if (macro_table == NULL) {
        fclose(as_fd);
        allocation_failure
    }

    filename = strcat(filename, ".am");
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
    read_next_part(as_fd, next_part);

    /* save macros in the macros table */
    if (strncmp(next_part, "macr", 4) == 0
            && isspace(next_part[4])) {
        read_next_part(as_fd, next_part);
        /* remove newline */
        next_part[strlen(next_part) - 1] = '\0';

        /* check if macro name is valid */
        if (is_macro_name_valid(next_part, macro_table, macro_counter)) {
            macro_table[macro_counter].name = strdup(next_part);
            ++macro_counter;
        }
        else {
            safe_free(next_part)
            fclose(as_fd);
            return 1;
        }

        /* run until macro is finished */
        while (strcmp(next_part, "endmacr") != 0) {
            read_next_part(as_fd, next_part);
            if (macro_content == NULL) {
                macro_content = strdup(next_part);
            } else {
                if (!(temp = realloc(macro_content,
                                    strlen(macro_content)
                                        + strlen(next_part) + 2))) {
                    safe_free(macro_content)
                    allocation_failure
                }
                macro_content = temp;
                strcat(macro_content, next_part);
            }
        }
        macro_table[macro_counter].content = strdup(next_part);
        ++macro_counter;
    }

    /* write next part to the new am file */
    if ((i = is_macro(next_part, macro_table, macro_counter))) {
        fwrite(macro_table[i].content, sizeof(macro_table[i].content),
               1, am_fd);
    }
    else {
        fwrite(next_part, sizeof(next_part), 1, am_fd);
    }

    /* free memory */
    for (i = 0; i < macro_counter; ++i) {
        safe_free(macro_table[i].name)
        safe_free(macro_table[i].content)
    }
    safe_free(macro_table)
    safe_free(next_part)

    return 0;
}


/* TODO pre_assembler needs to run on the passed files by adding a .as suffix
 * it will create a .am file after spreading macros
 * a macro start with macr and ends with end_macr */
int pre_assembler(int argc, char **argv) {
    unsigned int i; /* counter */
    FILE *fd; /* file pointer */
    char *filename = NULL; /* file name */

    for (i = 1; i < argc; i++) {
        if (!(filename = (char *)calloc(strlen(argv[i]) + 4, sizeof(char)))) {
            allocation_failure
        }
        /* open the file */
        argv[i] = strcat(argv[i], ".as");
        fd = fopen(argv[i], "r");
        if (fd == NULL) {
            fprintf(stderr, "Error: Could not open file %s\n", argv[i]);
            return 1;
        }

        macro_parser(fd, filename);
        /* close the file */
        fclose(fd);
    }

    return 0;
}
