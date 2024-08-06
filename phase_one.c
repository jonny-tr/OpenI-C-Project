#include "assembler.h"

#define LINE_SIZE 81
#define MAX_LABEL_LENGTH 31

/*updates for commit: wrote free_all, new allocation failure for phase 1, small derefrencing issues
todo:add frees before allocation failure
check max line number?*/

#define phase_one_allocation_failure \
    fprintf(stdout, "Memory allocation failed.\n"); \
    free_all(macro_table, *symbol_table, variable_table, command_table); \
    exit(EXIT_FAILURE);

#define CHECK_UNEXPECTED_COMMA(char_type, error_flag) \
    if ((char_type) == 1) { \
    fprintf(stdout, "Error: line %d in %s.\n       "\
                        "Unexpected comma.\n",\
                        line_counter, filename);\
        (error_flag) = 1; \
        break; \
    }

#define PRINT_OPERAND_ERROR(error_code) \
    switch(error_code) { \
        case -1: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "A command cannot be used as an operand.\n",\
                line_counter, filename);\
            break; \
        case -2: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "Invalid immediate operand, after # must follow a number.\n",\
                line_counter, filename);\
            break; \
        case -3: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "Invalid register.\n",\
                line_counter, filename);\
            break; \
        case -4: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "Macro cannot be used as an operand.\n",\
                line_counter, filename);\
            break; \
        case -5: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "Invalid label name, must start with a letter.\n",\
                line_counter, filename);\
            break; \
        case -6: \
            fprintf(stdout, "Error: line %d in %s.\n       "\
                "Invalid label name, must only contain letters and numbers.\n",\
                line_counter, filename);\
            break;                      \
        default: \
            break; \
    }


/**
 * @brief phase_one does the first pass on the file
 *         and builds the symbold and variables tables
 *
 * @param fd the file after pre_assembler
 * @param ic needs to be 0
 * @param dc needs to be 0
 * @param symbol_table the symbol table
 * @param variable_table the variable table
 * @param macro_table the macro table from pre_assembler
 * @param cmd_list_head the head of the command list
 *
 * @return 0 on success, -1 on failure;
*/
int phase_one(FILE *fd, char *filename, int *ic, int *dc,
              symbols_ptr *symbol_table, variable_ptr variable_table,
              command_ptr command_table, macro_ptr macro_table) {
    char line[LINE_SIZE], word[LINE_SIZE]; /* buffers */
    char *word_ptr, *label_temp_ptr = NULL; /* pointers */
    int label_flag = 0, error_flag = 0, expect_comma; /* flags: 1 on, 0 off */
    int i, cmnd, word_type, data_tmp, commas, operand_error,
        line_counter = 0; /* counters */
    int char_type; /* -1 line end, 0 word, 1 comma */
    command_ptr new_field = (command_ptr) malloc(sizeof(command_word)); /* command */

    if (new_field == NULL) {
        phase_one_allocation_failure
    }

    while (read_next_line(fd, line) != -1) {
        word_ptr = line;
        line_counter++;
        while ((char_type = get_next_word(line, word, &word_ptr)) != -1) {
            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
            word_type = get_word_type(word);
            switch (word_type) {
                case LABEL:
                    switch (is_valid_label(word, *symbol_table, macro_table)) {
                        case -1:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Cannot use a command as a label.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -2:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Label already exists.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -3:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Cannot use a macro as a label.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -4:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Cannot use a register as a label.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -5:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Label must start with a letter.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -6:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Label must only contain "
                                            "letters and numbers.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        case -7:
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Label is too long, max length of "
                                            "a label and its content shoult "
                                            "not exceed %d characters.\n",
                                    line_counter, filename,
                                    MAX_LABEL_LENGTH);
                            error_flag = 1;
                            break;
                        case 0: /* valid label */
                            if (label_flag == 1) {
                                fprintf(stdout, "Error: line %d in %s.\n"
                                                "       Cannot use two labels "
                                                "at once.\n",
                                        line_counter, filename);
                                error_flag = 1;
                            } else {
                                label_flag = 1;
                                as_strdup(&label_temp_ptr, word);
                            }
                            break;
                    } /* end label case */

                case DATA:
                    if (label_flag == 1) {
                        label_flag = 0;
                        if (add_symbol(*symbol_table, label_temp_ptr,
                                       *dc, "data") == -1) {
                            phase_one_allocation_failure
                        }
                    }
                    expect_comma = 0;
                    while ((char_type = get_next_word(line, word, &word_ptr)) != -1
                            && word[0] != '\0') {
                        if (expect_comma == 1) {
                            commas = comma_checker(line, &word_ptr);
                            if (commas == 0) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                "Missing comma.\n",
                                        line_counter, filename);
                                error_flag = 1;
                                break;
                            } else if (commas > 1) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                "Too many commas.\n",
                                        line_counter, filename);
                                error_flag = 1;
                                break;
                            } else expect_comma = 0;
                        }
                        data_tmp = get_data_int(word);
                        if (data_tmp == INVALID_INT) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Invalid data.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                        if (add_variable(&variable_table,
                                         twos_complement(data_tmp),
                                         *dc) == -1) {
                            phase_one_allocation_failure
                        }
                        (*dc)++;
                        expect_comma = 1;
                    }
                    break;

                case STRING:
                    if (label_flag == 1) {
                        label_flag = 0;
                        if (add_symbol(*symbol_table, label_temp_ptr, *dc, "data")
                            == -1) {
                            phase_one_allocation_failure
                        }
                    }
                    if (get_next_word(line, word, &word_ptr) != -1 && word[0] != '\0') {
                        if (word[0] == '"' && word[strlen(word) - 1] == '"')
                            for (i = 1; i < strlen(word) - 1; i++) { /*add the string without the quotes*/
                                if (add_variable(&variable_table, get_ascii_value(word[i]), *dc) ==
                                    -1) {
                                    phase_one_allocation_failure
                                }
                                (*dc)++;
                            }
                        else {
                            fprintf(stdout, "Error: line %d in %s.\n       " \
                                                    "Invalid string.\n", \
                                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                    }
                    break;
                case EXTERN:
                    expect_comma = 0;
                    while (get_next_word(line, word, &word_ptr) != -1 && word[0] != '\0') {
                        if (expect_comma == 1) {
                            commas = comma_checker(line, &word_ptr);
                            if (commas == 0) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Missing comma.\n",
                                                    line_counter, filename);
                                error_flag = 1;
                                break;
                            } else if (commas > 1) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Too many commas.\n",
                                                    line_counter, filename);
                                error_flag = 1;
                                break;
                            } else expect_comma = 0;
                        }
                        if (add_symbol(*symbol_table, word,
                                       INVALID_INT, "external") == -1) {
                            phase_one_allocation_failure
                        }
                        /*continue adding allocation faliure for add_symbol and then for add_variable
                        write free symbols and variable*/
                        expect_comma = 1;
                    }
                    break;
                case ENTRY:
                    if (get_next_word(line, word, &word_ptr) == 0) {
                        if (is_valid_label(word, *symbol_table, macro_table) == 0) {
                            if (add_symbol(*symbol_table, word, *ic + 100, "code") ==
                                -1) {
                                phase_one_allocation_failure
                            }
                            (*ic)++;
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Invalid label for Entry.\n",
                                                    line_counter, filename);
                            error_flag = 1;
                        }
                    } else {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Missing label for .entry "
                                                    "command.\n",
                                                    line_counter, filename);
                        error_flag = 1;
                    }
                    break;
                case COMMAND:
                    cmnd = is_valid_command(word);
                    if (cmnd == -1) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "%s: invalid command.\n",
                                                    line_counter, filename, word);
                        error_flag = 1;
                        break;
                    }
                    if (label_flag == 1) {
                        label_flag = 0;
                        if (add_symbol(*symbol_table, label_temp_ptr, (*ic + 100), "code") ==
                            -1) {
                            phase_one_allocation_failure
                        }
                        (*ic)++;
                    }
                    /*initialize new command_word*/
                    if (init_command_word(&command_table, &new_field) == -1) {
                        phase_one_allocation_failure
                    }
                    set_command_opcode(new_field, cmnd);
                    switch (cmnd) {
                        /*two operands*/
                        case 0: /*mov*/
                        case 1: /*cmp*/
                        case 2: /*add*/
                        case 3: /*sub*/
                        case 4: /*lea*/
                            /*first operand*/
                            if ((char_type = get_next_word(line, word, &word_ptr)) != -1) {
                                CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                                operand_error = is_valid_operand(word, macro_table);
                                if (operand_error == 1)
                                    set_addressing_method(word, new_field, 1);
                                else {
                                    PRINT_OPERAND_ERROR(operand_error);
                                    error_flag = 1;
                                    break;
                                }
                            } else {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Missing operands.\n",
                                                    line_counter, filename);
                                error_flag = 1;
                                break;
                            }
                            /*check for propper commas*/
                            if (comma_checker(line, &word_ptr) != 1) {
                                error_flag = 1;
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Invalid comma use.\n",
                                                    line_counter, filename);
                                break;
                            }

                            /*second oeprand*/
                            if ((char_type = get_next_word(line, word, &word_ptr)) != -1) {
                                operand_error = is_valid_operand(word, macro_table);
                                if (operand_error == 1)
                                    set_addressing_method(word, new_field, 2);
                                else {
                                    PRINT_OPERAND_ERROR(operand_error);
                                    error_flag = 1;
                                    break;
                                }
                            } else {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Missing destination operand.\n",
                                                    line_counter, filename);
                                error_flag = 1;
                                break;
                            }
                            break;

                        /*one operand*/
                        case 5: /*clr*/
                        case 6: /*not*/
                        case 7: /*inc*/
                        case 8: /*dec*/
                        case 9: /*jmp*/
                        case 10: /*bne*/
                        case 11: /*red*/
                        case 12: /*prn*/
                        case 13: /*jsr*/
                            /*only destination operand*/
                            if ((char_type = get_next_word(line, word, &word_ptr)) != -1) {
                                CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                                operand_error = is_valid_operand(word, macro_table);
                                if (operand_error == 1)
                                    set_addressing_method(word, new_field, 2);
                                else {
                                    PRINT_OPERAND_ERROR(operand_error);
                                    error_flag = 1;
                                    break;
                                }
                            } else {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                    "Missing operand.\n",
                                                    line_counter, filename);
                                error_flag = 1;
                                break;
                            }
                            break;
                            /* no operands - handeled in a different area */
                        case 14: /*rts*/
                        case 15: /*stop*/
                            break;
                        default:
                            fprintf(stdout, "Unknown error: line %d in %s.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                    } /* end of cmnd switch */
                case ERROR:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Invalid command.\n",
                            line_counter, filename);
                    error_flag = 1;
                    break;
                case -2: /* Space before : error */
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label cannot end with a whitespace.\n",
                            line_counter, filename);
                    error_flag = 1;
                    break;
                new_field->l = calc_l(new_field, cmnd);
                *ic += new_field->l;
            } /*end of word_type switch*/
        } /* end of line while */
            /* @shahar, this is not the end of line while, its the next_word loop,
             * is it supposed to include anything else? */
        /***label_flag = 0;?***/
    }
    end_phase_one_update_counter(*symbol_table, *ic);

    if (error_flag == 1) return -1;

    return 0;
}

/**
 * @brief Initializes a new command word and adds it to a linked list.
 *
 * @param head pointer to the head of the linked list.
 * @param ptr pointer to the newly created command word.
 *
 * @return 0 on success, -1 on failure.
 */
int init_command_word(command_ptr *head, command_ptr *ptr) {
    command_ptr new_node = (command_ptr) malloc(sizeof(command_word));

    if (new_node == NULL) return -1;

    new_node->are = 0x4; /* 0b100 in binary, automatically sets ARE to be 100 (only A) */
    new_node->dest_addr = 0x0; /* 0b0000 in binary */
    new_node->src_addr = 0x0;  /* 0b0000 in binary */
    new_node->opcode = 0x0;  /* 0b0000 in binary */
    new_node->l = 0x0; /* 0b000 in binary */
    new_node->next = NULL;

    if (*head == NULL) { /* initialize the list */
        *head = new_node;
    } else {
        command_ptr temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    *ptr = new_node;
    return 0;
}

/**
 * @brief Calculates the value of 'L' for a given command.
 *
 * @param field pointer to the command_word struct
 * @param cmnd the command code
 *
 * @return the value of 'l' based on the command
 */
int calc_l(command_word *field, int cmnd) {
    if (cmnd == 14 || cmnd == 15) return 0; /*command without operands*/
    /* one operand or both operands are registers: 0100 or 1000*/
    else if ((cmnd >= 5 && cmnd <= 13)
            || ((field->src_addr == 0x4 || field->src_addr == 0x8) /*0b0100||0b1000*/
                && (field->dest_addr == 0x4 || field->dest_addr == 0x8))) /*0b0100||0b1000*/ {
        return 1;
    } else return 2;
}

/**
 * @brief Sets the opcode of a command_word.
 *
 * @param field Pointer to a command_word.
 * @param command The command afer is_valid_command, determine opcode from.
 *
 * @return void
 */
void set_command_opcode(command_word *field, int command) {
    /* TODO: don't you want it to be a switch case? @shahar */
    if (command == 0) field->opcode = 0x0;       /* mov */
    else if (command == 1) field->opcode = 0x1;  /* cmp */
    else if (command == 2) field->opcode = 0x2;  /* add */
    else if (command == 3) field->opcode = 0x3;  /* sub */
    else if (command == 4) field->opcode = 0x4;  /* lea */
    else if (command == 5) field->opcode = 0x5;  /* clr */
    else if (command == 6) field->opcode = 0x6;  /* not */
    else if (command == 7) field->opcode = 0x7;  /* inc */
    else if (command == 8) field->opcode = 0x8;  /* dec */
    else if (command == 9) field->opcode = 0x9;  /* jmp */
    else if (command == 10) field->opcode = 0xA; /* bne */
    else if (command == 11) field->opcode = 0xB; /* red */
    else if (command == 12) field->opcode = 0xC; /* prn */
    else if (command == 13) field->opcode = 0xD; /* jsr */
    else if (command == 14) field->opcode = 0xE; /* rts */
    else if (command == 15) field->opcode = 0xF; /* stop */
}

/**
 * @brief Sets the addressing method in the command_word, used after is_valid_operand.
 *
 * @param operand The operand to be parsed, NULL if it is a command without operands.
 * @param field Pointer to the command_word struct.
 * @param src_dest 1 if source, 2 if destination.
 *
 * @return void
 */
void set_addressing_method(char *operand, command_word *field, int src_dest) {
    if (src_dest == 1) { /* source operand */
        if (operand == NULL) field->src_addr = 0x0;

            /* Immediate addressing */
        else if (operand[0] == '#') field->src_addr = 0x1;

            /* Indirect register addressing */
        else if (operand[0] == '*') field->src_addr = 0x4; /*0b0100*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            field->src_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else field->src_addr = 0x2; /*0b0010*/

    } else if (src_dest == 2) { /* destination operand */
        if (operand == NULL) field->dest_addr = 0x0;

            /* Immediate addressing or indirect register addressing */
        else if (operand[0] == '#' || operand[0] == '*')
            field->dest_addr = 0x1; /*0b0001*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            field->dest_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else field->dest_addr = 0x2; /*0b0010*/
    }
}

/**
 * @brief Checks if the given word is a valid operand.
 *
 * @param word The word to be checked.
 * @param macro_table The table of macros.
 *
 * @return 1 if the word is a valid operand, -1 command, -2 invalid immediate #,
 *         -3 invalid indirect register *, -4 macro, -5 invalid label 1st char, -6 invalid label
 *
 * @throws None.
 */
int is_valid_operand(char *word, macro_ptr macro_table) {
    int i;
    if (is_valid_command(word) != -1) {
        return -1; /*it is a command*/
    } else if (word[0] == '#') { /*needs to be a number constant*/
        /* Check for optional +- */
        i = 1;
        if (word[1] == '-' || word[1] == '+') i = 2;
        for (; word[i] != '\0'; i++)
            if (!isdigit(word[i])) {
                return -2;
            }
        if (i == 2 && !isdigit(word[2])) {
            return -2;
        }
    } else if (word[0] == '*') { /*needs to be a valid register*/
        if (!(strncmp(word, "r", 2) == 0 && strlen(word) == 3 && word[2] >= '0' && word[2] <= '7')) {
            return -3;
        }
    } else if (is_macro_name_valid(word, macro_table) == 2) {
        return -4;
    } else { /*needs to be a valid label*/
        if (isalpha(word[0]) == 0) {
            return -5;
        }
        for (i = 0; word[i] != '\0'; i++) {
            if (!isalnum(word[i])) {
                return -6;
            }
        }
    }
    return 1;
}

/**
 * @brief checks if the label name is valid
 *
 * @param word the name of the label
 * @param symbols_table_head the head of the symbols table
 * @param macro_table_head the head of the macro table
 *
 * @return -1 if command, -2 if label already exists, -3 if macro,
 *         -4 if register, -5 if doesn't start with a letter,
 *         -6 if isn't only letters and numbers, -7 if too long, 0 if valid
 */
int is_valid_label(char *word, symbols_ptr symbols_table_head,
                   macro_ptr macro_table_head) {
    int i = 0;
    char *used_registers[] = {"r0", "r1", "r2", "r3", "r4",
                              "r5", "r6", "r7"}; /* register names */
    symbols_list *current = symbols_table_head;

    /*remove colon*/
    while (word[i] != '\0') {
        if (word[i] == ':') {
            word[i] = '\0';
        }
        i++;
    }

    /*is it too long*/
    if (i > MAX_LABEL_LENGTH) return -7;

    /*is it a command*/
    if (is_valid_command(word) != -1) return -1;

    /*is it an existing label*/
    while (current != NULL) {
        if (strcmp(word, current->name) == 0) {
            return -2;
        }
        current = current->next;
    }

    /*is it a macro*/
    if (is_macro_name_valid(word, macro_table_head) == 2) return -3;

    /*is it a register*/
    for (i = 0; i < 8; i++) {
        if (strcmp(word, used_registers[i]) == 0) return -4;
    }

    /*does it start with a non-alpha character*/
    if (isalpha(word[0]) == 0) return -5;

    /* Check if label contains only alphanumeric characters */
    for (i = 0; word[i] != '\0'; i++) {
        if (!isalnum(word[i])) {
            return -6;
        }
    }

    return 0; /*valid*/
}

/*counter is IC*/

/**
 * @brief Adds a new symbol to the symbol table or initializes if needed
 *
 * @param head A pointer to the head of the symbol table linked list.
 * @param name The name of the symbol.
 * @param counter value of IC or DC.
 * @param type The type of the symbol: external / entry / data / code
 *
 * @return 0 on success, -1 on failure.
 */
int add_symbol(symbols_ptr head, char *name, int counter, char *type) {
    symbols_ptr temp, new_node = NULL;
    /* symbol nodes */

    new_node = (symbols_ptr) malloc(sizeof(symbols_list));
    if (new_node == NULL) return -1;

    as_strdup(&new_node->name, name);
    as_strdup(&new_node->type, type);

    new_node->counter = (strcmp(type, "external") == 0) ?
                        INVALID_INT : counter; /*IC or DC*/
    new_node->next = NULL;

    if (head == NULL) { /*initialize the list*/
        head = new_node;
    } else {
        temp = head;
        while (temp->next != NULL) temp = temp->next;
        temp->next = new_node;
    }

    return 0;
}

/**
 * @brief Updates the counter of all data symbols with IC+100.
 *
 * @param head A pointer to the head of the symbol linked list.
 * @param IC IC counter
 *
 * @return void
 */
void end_phase_one_update_counter(symbols_ptr head, int IC) {
    symbols_ptr temp = head;

    while (temp != NULL) {
        if (strcmp(temp->type, "data") == 0) temp->counter += IC + 100;
        temp = temp->next;
    }
}

/**
 * @brief Adds a new variable to the variable list or initializes it if needed.
 *
 * @param head A pointer to the head of the variable linked list.
 * @param content The content of the variable.
 * @param counter DC counter
 *
 * @return 0 on success, -1 on failure.
 */
int add_variable(variable_t **head, int content, int counter) {
    variable_ptr new_node = (variable_ptr) malloc(sizeof(variable_ptr));

    if (new_node == NULL) return -1;

    new_node->content = content;
    new_node->counter = counter; /*DC*/
    new_node->next = NULL;

    if (*head == NULL) { /*initialize the list*/
        *head = new_node;
    } else {
        variable_ptr temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return 0;
}

/**
 * @brief Parses a string and returns the integer value of the number it represents.
 *
 * @param word A pointer to the first character of the string to be parsed.
 *
 * @return The integer value of the number represented by the string.
 *
 * @throws None.
 */
int get_data_int(char *word) {
    int result = 0, sign = 1;

    /* Check for sign */
    if (*word == '-') {
        sign = -1;
        word++;
    } else if (*word == '+') {
        word++;
    }

    /* read the number */
    while (*word && *word != ' ' && *word != '\t'
           && *word != ',' && *word != '\0') {
        if (!isdigit(*word)) return INVALID_INT;
        result = result * 10 + (*word - '0');
        word++;
    }
    return sign * result;
}
