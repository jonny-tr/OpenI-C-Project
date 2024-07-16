#include "assembler.h"

#define next_check if (read_next_word(line, position, &next_word) == -1) { \
            fprintf(stdout, "Error: Failed to read from %s.\n", filename);\
            error_flag = 1;\
            break;\
            }

/**
 * @brief the function converts binary strings to octal
 * @param line the binary string
 * @return octal integer
 */
int binstr_to_octal(char *line) {
    int oct = 0, dec = 0, bin, i = 0; /* numbers and counterit */

    bin = atoi(line);

    while (bin != 0) {
        dec += (bin % 10) * pow(2, i);
        ++i;
        bin /= 10;
    }
    i = 1;

    while (dec != 0) {
        oct += (dec % 8) * i;
        dec /= 8;
        i *= 10;
    }
    return oct;
}

/**
 * @brief builds the object file
 * @param ob_fd pointer to the object file
 * @param filename the name of the file
 * @return 0 if successful, -1 otherwise
 */
int build_ob(FILE *ob_fd, char *filename) {
    char *line = NULL, *octal_line = NULL,
        *tmp_file = as_strcat(filename, ".tmp");
    int error_flag = 0, tmp_num;
    FILE *tmp_fd = fopen(tmp_file, "r");

    if (tmp_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", tmp_file);
        error_flag = 1;
    }

    while (error_flag == 0 && !feof(tmp_fd)) {
        if (read_next_line(tmp_fd, &line) != 0) {
            fprintf(stdout, "Error: Failed to read from %s.\n", tmp_file);
            error_flag = 1;
            break;
        }
        tmp_num = atoi(line);
    }

    if (tmp_fd != NULL && remove(tmp_file) != 0) {
        fprintf(stdout, "Error: Could not delete %s.\n", tmp_file);
        error_flag = 1;
    }

    safe_free(tmp_file)

    if (error_flag) return -1;

    return 0;
}

/**
 * @brief updates the type of the symbol to be entry
 * @param symbol_table_head pointer to the symbol table head
 * @param word name of the symbol to be updated
 * @return 0 if successful, -1 otherwise
 */
int entry_update(symbol_ptr symbol_table_head, char *word) {
    symbol_ptr current = symbol_table_head;

    while (current != NULL) {
        if (strcmp(word, current->name) == 0) {
            current->type = "entry";
            return 0;
        }
        current = current->next;
    }

    fprintf(stdout, "Error: Symbol %s, defined as entry, not found.\n", word);
    return -1;
}

/**
 * @brief TODO fill details in the end
 * @param fd pointer to the file
 * @param filename name of the file
 * @param symbol_table pointer to the symbol table
 * @param ext_ic expected ic value
 * @param dc value of dc
 * @return 0 if successful, -1 otherwise
 */
int phase_two(FILE *fd, char *filename, symbol_ptr symbol_table,
              int ext_ic, int dc) {
    char *line = NULL, *next_word = NULL, *ob_file = NULL, *ext_file = NULL,
        *ent_file = NULL; /* strings and filenames */
    int ic = 0, *position = 0, error_flag = 0, temp_flag = 0;
    FILE *ob_fd = NULL, *ext_fd = NULL, *ent_fd = NULL;

    ob_file = as_strcat(filename, ".ob");
    ext_file = as_strcat(filename, ".ext");
    ent_file = as_strcat(filename, ".ent");

    while (read_next_line(fd, &line) != -1 || !feof(fd)) {
        next_check
        if (next_word[strlen(next_word) - 1] == ':') continue;
        if ((strcmp(next_word, ".data") == 0)
                || (strcmp(next_word, ".string") == 0)
                || (strcmp(next_word, ".extern") == 0))
            continue;

        if (strcmp(next_word, ".entry") == 0) {
            while (temp_flag == 0) {
                next_check
                if (strcmp(next_word, ".end") == 0) {
                    temp_flag = 1;
                    continue;
                }
                if (entry_update(symbol_table, next_word) == -1) {
                    error_flag = 1;
                    break;
                }
            }
            continue;
        }
    }

    if (ic != ext_ic) {
        fprintf(stdout, "Unknown error encountered during execution.\n"
                        "Review file %s.\n", filename);
        error_flag = 1;
    }

    if (error_flag == 0) {
        ob_fd = fopen(ob_file, "w");
        ext_fd = fopen(ext_file, "w");
        ent_fd = fopen(ent_file, "w");
        if (ob_fd == NULL || ext_fd == NULL || ent_fd == NULL) {
            fprintf(stdout, "Error: Could not open file %s.\n", filename);
            error_flag = 1;
            goto cleanup;
        }
        fprintf(ob_fd, "%d %d\n", ic, dc);
        build_ob(ob_fd, filename);
        build_ext(ext_fd, symbol_table);
        build_ent(ent_fd, symbol_table);
    }

    cleanup:
    if (ob_fd != NULL) fclose(ob_fd);
    if (ext_fd != NULL) fclose(ext_fd);
    if (ent_fd != NULL) fclose(ent_fd);

    safe_free(line)
    safe_free(next_word)
    safe_free(ob_file)
    safe_free(ext_file)
    safe_free(ent_file)

    if (error_flag) {
        if (ob_fd != NULL) remove(ob_file);
        if (ext_fd != NULL) remove(ext_file);
        if (ent_fd != NULL) remove(ent_file);
        return -1;
    }

    return 0;
}
