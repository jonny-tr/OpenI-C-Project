#include "assembler.h"

/**
 * @brief builds the ent file
 * @param ent_fd a pointer to the ent file
 * @param symbol_head a pointer to the symbol table
 * @return 0
 */
void build_ent(FILE *ent_fd, symbol_ptr symbol_head) {
    symbol_ptr current = symbol_head;

    while (current != NULL) {
        if (strcmp(current->type, "entry") == 0) {
            fprintf(ent_fd, "%s %d\n", current->name, current->counter);
        }
        current = current->next;
    }
}

/**
 * @brief builds the object file
 * @param ob_fd pointer to the object file
 * @param command_word pointer to the command list
 * @param ic the instruction counter
 * @param dc the data counter
 * @return 0 if successful, -1 otherwise
 */
void build_ob(FILE *ob_fd, command_ptr command_head, variable_ptr variable_head,
             int ic, int dc) {
    int i = 100, mask = 0x7FFF; /* counter and mask */
    command_ptr current_cmd = command_head;   /* current command */
    variable_ptr current_var = variable_head; /* current variable */

    fprintf(ob_fd, "%4d %d\n", ic, dc);

    while (current_cmd != NULL) {
        fprintf(ob_fd, "%04d %05o\n", i, command_to_num(current_cmd));
        current_cmd = current_cmd->next;
        i++;
    }

    while (current_var != NULL) {
        fprintf(ob_fd, "%04d %05o\n", i, (current_var->content & mask));
        current_var = current_var->next;
        i++;
    }
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
 * @return symbol counter, -1 if not a symbol, -2 on allocation failure
 */
int is_symbol(char *name, symbol_ptr symbols_head, command_ptr are,
              FILE **ext_fd, char *ext_file, const int line_num) {
    symbol_ptr current = symbols_head;

    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            if (strcmp(current->type, "entry") == 0)
                are->are = 2;
            else if (strcmp(current->type, "external") == 0) {
                if (*ext_fd == NULL) {
                    *ext_fd = fopen(ext_file, "w");
                    if (*ext_fd == NULL) {
                        fprintf(stdout, "Error: Failed to open %s.\n",
                                ext_file);
                        return -2;
                    }
                }
                fprintf(*ext_fd, "%-4s %04d\n", current->name, line_num);
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
 * @param current_cmd the current command in the list
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
int update_command_list(command_ptr *current_cmd, char *word, char **word_ptr,
                        char *filename, symbol_ptr symbol_head,
                        FILE **ext_fd, char *ext_file, const int line_num) {
    int num, address_flag; /* counter and flag */
    command_ptr src_node = NULL, dest_node = NULL;

    src_node = (command_ptr) calloc(1, sizeof(command_t));
    if (src_node == NULL)
        return -2;

    /* insert src_node to the linked list */
    src_node->next = (*current_cmd)->next;
    (*current_cmd)->next = src_node;

    /* both source and destination are registers */
    if (((*current_cmd)->l == 1)
        && ((*current_cmd)->opcode >= 0 && (*current_cmd)->opcode <= 4)) {
        src_node->are = 4;
        src_node->dest_addr = atoi(&word[strlen(word) - 1]);
        get_next_word(word, word_ptr); /* skip comma */
        if (get_next_word(word, word_ptr) == -1) {
            fprintf(stdout, "Error: Failed to read from %s.\n", filename);
            return -1;
        }
        src_node->dest_addr = src_node->dest_addr | (atoi(word) << 3);
        src_node->src_addr = (atoi(&word[strlen(word) - 1]) >> 1);
        (*current_cmd) = src_node->next;
        return 0;
    }

    if ((*current_cmd)->l == 1) address_flag = (*current_cmd)->dest_addr;
    else address_flag = (*current_cmd)->src_addr;

    switch (address_flag) {
        case 1: /* immediate address */
            src_node->are = 4;
            num = atoi(&word[1]);
            src_node->dest_addr = num;
            src_node->src_addr = (num >> 4);
            src_node->opcode = (num >> 8);
            break;
        case 2: /* direct address */
            num = is_symbol(word, symbol_head, src_node, ext_fd,
                            ext_file, line_num);
            if (num == -1) { /* not a symbol */
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Label %s not found.\n",
                        line_num, filename, word);
                return -1;
            } else if (num == -2) /* allocation failure */
                return -2;
            src_node->dest_addr = num;
            src_node->src_addr = (num >> 4);
            src_node->opcode = (num >> 8);
            break;
        case 4: /* indirect register address */
        case 8: /* direct register address */
            src_node->are = 4;
            src_node->dest_addr = (atoi(&word[strlen(word) - 1]) << 3);
            src_node->src_addr = (atoi(&word[strlen(word) - 1]) >> 1);
            break;
        default:
            break;
    }

    if ((*current_cmd)->l == 1) {
        (*current_cmd) = src_node->next;
        return 0;
    }

    dest_node = (command_ptr) calloc(1, sizeof(command_t));
    if (dest_node == NULL)
        return -2;

    /* insert dest_node to the linked list */
    dest_node->next = src_node->next;
    src_node->next = dest_node;

    get_next_word(word, word_ptr); /* skip comma */
    if (get_next_word(word, word_ptr) == -1) {
        fprintf(stdout, "Error: Failed to read from %s.\n", filename);
        return -1;
    }

    switch ((*current_cmd)->dest_addr) {
        case 1: /* immediate address */
            dest_node->are = 4;
            num = atoi(&word[1]);
            dest_node->dest_addr = num;
            dest_node->src_addr = (num >> 4);
            dest_node->opcode = (num >> 8);
            break;
        case 2: /* direct address */
            num = is_symbol(word, symbol_head, src_node, ext_fd,
                            ext_file, line_num);
            if (num == -1) { /* not a symbol */
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Label %s not found.\n",
                        line_num, filename, word);
                return -1;
            } else if (num == -2) /* allocation failure */
                return -2;
            dest_node->dest_addr = num;
            dest_node->src_addr = (num >> 4);
            dest_node->opcode = (num >> 8);
            break;
        case 4: /* indirect register address */
        case 8: /* direct register address */
            dest_node->are = 4;
            dest_node->dest_addr = (atoi(&word[strlen(word) - 1]));
            break;
        default:
            break;
    }

    (*current_cmd) = dest_node->next;
    return 0;
}

/**
 * @brief updates the type of the symbol to be entry
 * @param symbol_head pointer to the symbol table head
 * @param word name of the symbol to be updated
 * @return 0 if successful, -1 otherwise
 */
int update_entry(symbol_ptr symbol_head, char *word, char *filename,
                 int line_num) {
    symbol_ptr current = symbol_head;

    while (current != NULL) {
        if (strcmp(word, current->name) == 0) {
            current->type = "entry";
            return 0;
        }
        current = current->next;
    }

    fprintf(stdout, "Error: line %d in %s.\n       "
                    "Symbol %s, defined as entry, not found.\n",
            line_num, filename, word);

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
int phase_two(FILE *am_fd, char *filename, symbol_ptr symbol_head,
              variable_ptr variable_head, command_ptr command_head,
              int expected_ic, int dc) {
    char line[LINE_SIZE] = {0}, word[LINE_SIZE] = {0}, *am_file,
        *ob_file = NULL, *ext_file = NULL, *ent_file = NULL, *word_ptr = NULL;
        /* strings and filenames */
    int line_num = 0, ic = 0, error_flag = 0, allocation_flag = 0, word_flag,
            ent_flag = 0;                     /* counters and flags */
    FILE *ob_fd = NULL, *ext_fd = NULL, *ent_fd = NULL; /* file pointers */
    command_ptr current_cmd = command_head;             /* command pointer */

    am_file = as_strcat(filename, ".am");
    ob_file = as_strcat(filename, ".ob");
    ext_file = as_strcat(filename, ".ext");
    ent_file = as_strcat(filename, ".ent");

    while (read_next_line(am_fd, line) != -1) {
        if (error_flag) break;
        line_num++;
        word_ptr = line;
        next_word_check

        if (word[strlen(word) - 1] == ':') {
            next_word_check
        } /* skip label */

        if ((strcmp(word, ".data") == 0)
            || (strcmp(word, ".string") == 0)
            || (strcmp(word, ".extern") == 0)) { /* TODO ensure the next word was not already used as a lbael */
            continue; /* next line */
        } else if (strcmp(word, ".entry") == 0) {
            ent_flag = 1;
                        /* update labels in the symbol table */
            while ((word_flag = get_next_word(word, &word_ptr)) != -1) {
                if (word_flag == 1) continue; /* skip comma */
                if (update_entry(symbol_head, word,
                                 am_file, line_num) == -1)
                    error_flag = 1;
            }
            continue; /* next line */
        } else {
            ic += current_cmd->l + 1;
            if (current_cmd->l == 0) {
                continue; /* next line */
            }
            next_word_check
            /* add new nodes to the command list */
            switch (update_command_list(&current_cmd, word, &word_ptr,
                                        am_file, symbol_head, &ext_fd,
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
        }
    } /* finish next_line loop */

    if (error_flag == 0 && ic != expected_ic) {
        fprintf(stdout, "Unknown error encountered during execution.\n        "
                        "Review file %s.\n",
                am_file);
        error_flag = 1;
    }

    if (error_flag == 0) {
        ob_fd = fopen(ob_file, "w");

        if (ob_fd == NULL) {
            fprintf(stdout, "Error: Could not create an output file "
                            "%s.\n",
                    ob_file);
            error_flag = 1;
            goto cleanup;
        }

        build_ob(ob_fd, command_head, variable_head,expected_ic, dc);

        if (ent_flag) {
            ent_fd = fopen(ent_file, "w");
            if (ent_fd == NULL) {
                fprintf(stdout, "Error: Could not create an entry file "
                                "%s.\n",
                        ent_file);
                error_flag = 1;
                goto cleanup;
            }
            build_ent(ent_fd, symbol_head);
        }
    }

    cleanup:
    if (ob_fd != NULL) fclose(ob_fd);
    if (ext_fd != NULL) fclose(ext_fd);
    if (ent_fd != NULL) fclose(ent_fd);

    safe_free(ob_file)
    safe_free(ext_file)
    safe_free(ent_file)
    
    if (allocation_flag) {
        allocation_failure
    }

    if (error_flag)
        return -1;

    return 0;
}
