#include "assembler.h"

/**
 * @brief builds the ent file
 * @param ent_fd a pointer to the ent file
 * @param symbol_head a pointer to the symbol table
 * @return 0
 */
int build_ent(FILE *ent_fd, symbols_ptr symbol_head) {
    symbols_ptr current = symbol_head;

    while (current != NULL) {
        if (strcmp(current->type, "entry") == 0) {
            fprintf(ent_fd, "%s %d\n", current->name, current->counter);
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief builds the ext file
 * @param ext_fd a pointer to the ext file
 * @param symbol_head a pointer to the symbol table
 * @return 0
 */
int build_ext(FILE *ext_fd, symbols_ptr symbol_head) {
    symbols_ptr current = symbol_head;

    while (current != NULL) {
        if (strcmp(current->type, "extenal") == 0) {
            fprintf(ext_fd, "%s %d\n", current->name, current->counter);
        }
        current = current->next;
    }

    return 0;
}

/**
 * @brief builds the object file
 * @param ob_fd pointer to the object file
 * @param command_word pointer to the command list
 * @param ic the instruction counter
 * @param dc the data counter
 * @return 0 if successful, -1 otherwise
 */
int build_ob(FILE *ob_fd, command_ptr command_head, variable_ptr variable_head,
             int ic, int dc) {
    int i; /* counter */
    command_ptr current_cmd = command_head; /* current command */
    variable_ptr current_var = variable_head; /* current variable */

    fprintf(ob_fd, "   %d %d\n", ic, dc);

    for (i = 100; i <= ic + 100; i++) {
        fprintf(ob_fd, "%04d %05o\n", i, command_to_num(*current_cmd));
        current_cmd = current_cmd->next;
    }

    if (current_cmd->next != NULL) return -1;

    for (i = 100; i <= dc + 100; i++) {
        fprintf(ob_fd, "%04d %05o\n", current_var->counter,
                current_var->content); /* TODO: twos_complement? double check value printed */
        current_var = current_var->next;
    }

    if (current_var->next != NULL) return -1;

    return 0;
}

/**
 * @brief checks if the symbol is in the symbol table and returns its counter,
 *        prints external symbols to the specified external file
 * @param name name of the symbol to be checked
 * @param symbols_head a pointer to the symbol table
 * @param are a pointer to the are command
 * @param ext_fd a pointer to the external file
 * @param ext_file the name of the external file
 * @param line_num the line number of the command
 * @return the counter of the symbol, -1 if an error occurred
 */
int is_symbol(char *name, symbols_ptr symbols_head, command_ptr are,
              FILE **ext_fd, char *ext_file, const int line_num) {
    symbols_ptr current = symbols_head;

    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            if (strcmp(current->type, "entry") == 0) are->are = 2;
            else if (strcmp(current->type, "external") == 0) {
                if (*ext_fd == NULL) {
                    *ext_fd = fopen(ext_file, "w");
                    if (*ext_fd == NULL) {
                        fprintf(stdout, "Error: Failed to open %s.\n",
                                ext_file);
                        return -1;
                    }
                }
                fprintf(*ext_fd, "%3s %04d\n", current->name, line_num);
                are->are = 1;
            }
            return current->counter;
        }
        current = current->next;
    }

    return -1;
}

/**
 * @brief the function updates the command list and adds the new command words
 * @param command_list the list of commands
 * @param word the current word in the line
 * @param line the current line
 * @param position the current position in the line
 * @param filename the name of the file
 * @param symbol_head the list of symbols
 * @param ext_fd a pointer to the external file
 * @param ext_file the name of the external file
 * @param line_num the line number
 * @return 0 if successful, -1 on failure, -2 on allocation failure
 */
int update_command_list(command_ptr command_list, char *word, char **word_ptr,
                        char *filename, symbols_ptr symbol_head,
                        FILE **ext_fd, char *ext_file, const int line_num) {
    int num;
    char *next_word = NULL;
    command_ptr current = command_list, new_node = NULL;

    new_node = (command_ptr) calloc(1, sizeof(command_word));
    if (new_node == NULL) return -2;

    new_node->next = current->next;
    current->next = new_node;

    /* both source and destination are registers */
    if ((current->src_addr == 0
            || current->src_addr == 4
            || current->src_addr == 8)
                && (current->dest_addr == 0
                    || current->dest_addr == 4
                    || current->dest_addr == 8)) {
        new_node->are = 4;
        new_node->dest_addr = atoi(&word[strlen(word) - 1]);
        if (get_next_word(word, word_ptr) == -1
                /*read_next_word(line, position, next_word) == -1*/) {
            fprintf(stdout, "Error: Failed to read from %s.\n", filename);
            return -1;
        }
        new_node->dest_addr = new_node->dest_addr | (atoi(next_word) << 3);
        new_node->src_addr = (atoi(&word[strlen(word) - 1]) >> 1);

        return 0;
    }

    switch (current->src_addr) {
        case 1: /* immediate address */
            new_node->are = 4;
            num = twos_complement(atoi(&word[1]));
            new_node->dest_addr = num;
            new_node->src_addr = (num >> 4);
            new_node->opcode = (num >> 8);
            break;
        case 2: /* direct address */
            if ((num = is_symbol(word, symbol_head, new_node, ext_fd,
                                 ext_file, line_num)) == -1)
                return -1;
            new_node->dest_addr = num;
            new_node->src_addr = (num >> 4);
            new_node->opcode = (num >> 8);
            break;
        case 4: /* indirect register address */
        case 8: /* direct register address */
            new_node->are = 4;
            new_node->dest_addr = (atoi(&word[strlen(word) - 1]) << 3);
            new_node->src_addr = (atoi(&word[strlen(word) - 1]) >> 1);
            break;
        default:
            break;
    }

    new_node = (command_ptr) calloc(1, sizeof(command_word));
    if (new_node == NULL) return -2;

    new_node->next = current->next;
    current->next = new_node;

    switch (current->dest_addr) {
        case 1: /* immediate address */
            new_node->are = 4;
            num = twos_complement(atoi(&word[1]));
            new_node->dest_addr = num;
            new_node->src_addr = (num >> 4);
            new_node->opcode = (num >> 8);
            break;
        case 2: /* direct address */
            if ((num = is_symbol(word, symbol_head, new_node, ext_fd,
                                 ext_file, line_num)) == -1)
                return -1;
            new_node->dest_addr = num;
            new_node->src_addr = (num >> 4);
            new_node->opcode = (num >> 8);
            break;
        case 4: /* indirect register address */
        case 8: /* direct register address */
            new_node->are = 4;
            new_node->dest_addr = (atoi(&word[strlen(word) - 1]));
            break;
        default:
            break;
    }

    return 0;
}

/**
 * @brief updates the type of the symbol to be entry
 * @param symbol_table_head pointer to the symbol table head
 * @param word name of the symbol to be updated
 * @return 0 if successful, -1 otherwise
 */
int entry_update(symbols_ptr symbol_table_head, char *word) {
    symbols_ptr current = symbol_table_head;

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
 * @param am_fd pointer to the .am file
 * @param filename name of the file
 * @param symbol_head pointer to the symbol table
 * @param expected_ic expected ic value
 * @param dc value of dc
 * @return 0 if successful, -1 otherwise
 */
int phase_two(FILE *am_fd, char *filename, symbols_ptr symbol_head,
              variable_ptr variable_head, command_ptr command_head,
              int expected_ic, int dc) {
    char line[LINE_SIZE] = {0}, word[LINE_SIZE] = {0}, *ob_file = NULL, *ext_file = NULL,
        *ent_file = NULL, *word_ptr = NULL; /* strings and filenames */
    int line_num = 0, ic = 0, error_flag = 0, allocation_flag = 0,
        ent_flag = 0, ext_flag = 0; /* counters and flags */
    FILE *ob_fd = NULL, *ext_fd = NULL, *ent_fd = NULL; /* file pointers */
    command_ptr current_cmd = command_head, tmp_cmd = NULL; /* pointers */

    ob_file = as_strcat(filename, ".ob");
    ext_file = as_strcat(filename, ".ext");
    ent_file = as_strcat(filename, ".ent");

    while (read_next_line(am_fd, line) != -1
           && !feof(am_fd)
           && current_cmd->next != NULL) {
        line_num++;
        word_ptr = line;
        next_word_check

        if (word[strlen(word) - 1] == ':') {
            next_word_check
        } /* skip label */

        if ((strcmp(word, ".data") == 0)
            || (strcmp(word, ".string") == 0)) {
            continue; /* skip */
        } else if (strcmp(word, ".extern") == 0) {
            ext_flag = 1;
            continue;
        } else if (strcmp(word, ".entry") == 0) {
            ent_flag = 1;
            /* update labels in the symbol table */
            while (get_next_word(word, &word_ptr) != -1) {
                if (entry_update(symbol_head, word) == -1) {
                    error_flag = 1;
                    break;
                }
            }
            continue;
        } else {
            ic += current_cmd->l;
            if (current_cmd->l == 1) continue;
            tmp_cmd = current_cmd->next;
            next_word_check
            /* add new nodes to the command list */
            switch (update_command_list(current_cmd, word, &word_ptr,
                                    filename, symbol_head, &ext_fd,
                                    ext_file, line_num)) {
                case -1:
                    error_flag = 1;
                    break;
                case -2:
                    error_flag = 1;
                    allocation_flag = 1;
                    goto cleanup;
                default:
                    break;
            }
            current_cmd->next = tmp_cmd;
            current_cmd = current_cmd->next;
        }
    }

    /*if (ic + dc != expected_ic) {
        fprintf(stdout, "Unknown error encountered during execution.\n"
                        "Review file %s.\n", filename);
        error_flag = 1;
    }*/

    if (error_flag == 0) {
        ob_fd = fopen(ob_file, "w");

        if (ob_fd == NULL) {
            fprintf(stdout, "Error: Could not create an output file for "
                            "%s.\n",
                    filename);
            error_flag = 1;
            goto cleanup;
        }

        if (build_ob(ob_fd, command_head, variable_head, ic, dc) == -1) {
            error_flag = 1;
            allocation_flag = 1;
            goto cleanup;
        }

        if (ent_flag) {
            ent_fd = fopen(ent_file, "w");
            if (ent_fd == NULL) {
                fprintf(stdout, "Error: Could not create an entry file for "
                                "%s.\n",
                        filename);
                error_flag = 1;
                goto cleanup;
            }
            build_ent(ent_fd, symbol_head);
        }

        if (ext_flag) {
            ext_fd = fopen(ext_file, "w");
            if (ext_fd == NULL) {
                fprintf(stdout, "Error: Could not create an external file for "
                                "%s.\n",
                        filename);
                error_flag = 1;
                goto cleanup;
            }
            /*build_ext(ext_fd, symbol_head); TODO: delete? built in is_symbol*/
        }
    }

    cleanup:
    if (ob_fd != NULL) fclose(ob_fd);
    if (ext_fd != NULL) fclose(ext_fd);
    if (ent_fd != NULL) fclose(ent_fd);

    safe_free(ob_file)
    safe_free(ext_file)
    safe_free(ent_file)

    /* remove files in case there was an error and they were still created */
    if (error_flag) {
        if (ob_fd != NULL) remove(ob_file);
        if (ext_fd != NULL) remove(ext_file);
        if (ent_fd != NULL) remove(ent_file);
    }

    if (allocation_flag) {
        allocation_failure
    }

    if (error_flag) return -1;

    return 0;
}
