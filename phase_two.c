#include "assembler.h"

#define next_word_check if (read_next_word(line, position, &next_word) \
                == -1) { \
            fprintf(stdout, "Error: Failed to read from %s.\n", filename);\
            error_flag = 1;\
            break;\
            }

/**
 * @brief builds the ent file
 * @param ent_fd a pointer to the ent file
 * @param symbol_table a pointer to the symbol table
 * @return 0
 */
int build_ent(FILE *ent_fd, symbol_ptr symbol_table) {
    symbol_ptr current = symbol_table;

    while (current != NULL) {
        if (strcmp(current->type, "entry") == 0) {
            fprintf(ent_fd, "%s %d\n", current->name, current->counter);
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief builds the object file
 * @param ob_fd pointer to the object file
 * @param filename the name of the file
 * @param ic the instruction counter
 * @param dc the data counter
 * @return 0 if successful, -1 otherwise
 */
int build_ob(FILE *ob_fd, char *filename, int ic, int dc) {
    char *line = NULL, *tmp_file = NULL; /* strings */
    int i, error_flag = 0; /* counter and error flag */
    FILE *tmp_fd = NULL; /* file pointer */

    if ((tmp_file = as_strcat(filename, ".tmp")) == NULL) {
        return -1;
    }

    tmp_fd = fopen(tmp_file, "r");
    if (tmp_fd == NULL) {
        fprintf(stdout, "Error: Could not open file %s.\n", tmp_file);
        error_flag = 1;
    }

    fprintf(ob_fd, "   %d %d\n", ic, dc);

    for (i = 100; i < ic + 100; i++) {
        if (read_next_line(tmp_fd, &line) != 0) {
            fprintf(stdout, "Error: Failed to read from %s.\n", tmp_file);
            error_flag = 1;
            break;
        }
        fprintf(ob_fd, "%04d %05d\n", i, binstr_to_octal(line));
        if (error_flag == 1) break;
    }

    if (tmp_fd != NULL && remove(tmp_file) != 0) {
        fprintf(stdout, "Error: Could not delete %s.\n", tmp_file);
        error_flag = 1;
    }

    fclose(tmp_fd);
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
 * @brief phase two of the assembler, builds the outpu files
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
    int ic = 0, l = 0, *position = 0, error_flag = 0, allocation_flag = 0;
        /* counters and flags */
    FILE *ob_fd = NULL, *ext_fd = NULL, *ent_fd = NULL; /* file pointers */

    ob_file = as_strcat(filename, ".ob");
    ext_file = as_strcat(filename, ".ext");
    ent_file = as_strcat(filename, ".ent");

    while (read_next_line(fd, &line) != -1 || !feof(fd)) {
        next_word_check
        if ((next_word[strlen(next_word) - 1] == ':') /* label */
                || (strcmp(next_word, ".data") == 0)
                || (strcmp(next_word, ".string") == 0)
                || (strcmp(next_word, ".extern") == 0))
            continue;

        if (strcmp(next_word, ".entry") == 0) {
            next_word_check
            if (entry_update(symbol_table, next_word) == -1) {
                error_flag = 1;
                break;
            }
            continue;
        }

/*        if ((l = calc_l(line, &ic) == -1)) {
            error_flag = 1;
            break;
        }*/

        ic += 1;
        if (l == 0) continue;
        ic += l+1;

        next_word_check
        /* is extern label? if so, write to ext file */


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
            fprintf(stdout, "Error: Could not create an output file for %s.\n",
                    filename);
            error_flag = 1;
            goto cleanup;
        }
        if (build_ob(ob_fd, filename, ic, dc) == -1) {
            error_flag = 1;
            allocation_flag = 1;
            goto cleanup;
        }
        /*build_ext(ext_fd, symbol_table);*/
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

    if (allocation_flag) {
        allocation_failure
    }


    return 0;
}
