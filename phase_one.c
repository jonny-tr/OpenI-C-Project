#include "assembler.h"

#define MAX_LABEL_LENGTH 31

/*updates for commit:  L_SPACE instead of -2
TODO: 
*/

#define phase_one_allocation_failure                                    \
    fprintf(stdout, "Memory allocation failed.\n");                     \
    free_all(*macro_head, *symbol_head, *variable_head, *command_head); \
    exit(EXIT_FAILURE);

#define CHECK_UNEXPECTED_COMMA(char_type, error_flag)    \
    if ((char_type) == 1) {                              \
        fprintf(stdout, "Error: line %d in %s.\n       " \
                        "Unexpected comma.\n",           \
                line_counter, filename);                 \
        (error_flag) = 1;                                \
        break;                                           \
    }

#define PRINT_OPERAND_ERROR(error_code)                                               \
    switch (error_code) {                                                             \
    case O_COMMAND:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "A command cannot be used as an operand.\n",                  \
                line_counter, filename);                                              \
        break;                                                                        \
    case O_IMMEDIATE:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid immediate operand, after # must follow a number.\n", \
                line_counter, filename);                                              \
        break;                                                                        \
    case O_INDIRECT:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid register.\n",                                        \
                line_counter, filename);                                              \
        break;                                                                        \
    case O_MACRO:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Macro cannot be used as an operand.\n",                      \
                line_counter, filename);                                              \
        break;                                                                        \
    case O_START:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid label name, must start with a letter.\n",            \
                line_counter, filename);                                              \
        break;                                                                        \
    case O_CHAR:                                                                          \
        fprintf(stdout, "Error: line %d in %s.\n       "                              \
                        "Invalid label name, must only contain "                      \
                        "letters and numbers.\n",                                     \
                line_counter, filename);                                              \
        break;                                                                        \
    default:                                                                          \
        break;                                                                        \
    }

/**
 * @brief phase_one does the first pass on the file
 *         and builds the symbols and variables tables
 * @param am_fd pointer to the .am file
 * @param filename the name of the file
 * @param ic instruction counter
 * @param dc data counter
 * @param symbol_head the symbol table
 * @param variable_head the variable table
 * @param command_head the command list
 * @param macro_head the macro table from pre_assembler
 * @return 0 on success, -1 on failure;
 */
int phase_one(FILE *am_fd, char *filename, int *ic, int *dc,
              symbol_ptr *symbol_head, variable_ptr *variable_head,
              command_ptr *command_head, macro_ptr *macro_head) {
    char line[LINE_SIZE] = {0}, word[LINE_SIZE] = {0};            /* buffers */
    char *word_ptr, *label_temp_ptr = NULL;                       /* pointers */
    int label_flag = 0, error_flag = 0, expect_comma, /* flags */
        i, cmnd, word_type, data_tmp, commas, operand_error,
        line_counter = 0, mask = 0x7FFF, /* counters */
        char_type; /* -1 line end, 0 word, 1 comma */
    command_ptr new_field = (command_ptr) calloc(1, sizeof(command_t)); /* command */

    if (new_field == NULL) {
        phase_one_allocation_failure
    }

    while (read_next_line(am_fd, line) != -1) {
        word_ptr = line;
        line_counter++;
        char_type = get_next_word(word, &word_ptr);
        if (char_type == L_SPACE) {
            fprintf(stdout, "Error: line %d in %s.\n       "
                            "':' must be adjacent to a label.\n",
                    line_counter, filename);
            error_flag = 1;
            word_type = FINISH;
        } else {
            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
            word_type = get_word_type(word);
        }
        if (word_type == LABEL) {
            switch (is_valid_label(word, *symbol_head, *macro_head)) {
                case L_COMMAND:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a command as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_EXISTS:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label already exists.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_MACRO:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a macro as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_REGISTER:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Cannot use a register as a label.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_START:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label must start with a letter.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_CHAR:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label must only contain "
                                    "letters and numbers.\n",
                            line_counter, filename);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_LONG:
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Label is too long, max length of "
                                    "a label and its content shoult "
                                    "not exceed %d characters.\n",
                            line_counter, filename, MAX_LABEL_LENGTH);
                    error_flag = 1;
                    word_type = FINISH;
                    break;
                case L_VALID: /* valid label */
                    as_strdup(&label_temp_ptr, word);
                    label_flag = 1;
                    char_type = get_next_word(word, &word_ptr);
                    if (char_type == -1) {
                        label_flag = 0;
                        error_flag = 1;
                        word_type = MISSING_CODE;
                        break;
                    }
                    CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                    word_type = get_word_type(word);
                    break;
            }
        }

        switch (word_type) {
            case LABEL:
                fprintf(stdout, "Error: line %d in %s.\n"
                                "       Cannot use two labels "
                                "at once.\n",
                        line_counter, filename);
                error_flag = 1;
                break;

            case DATA:
                if (label_flag == 1) {
                    label_flag = 0;
                    if (add_symbol(symbol_head, label_temp_ptr, *dc,
                                   "data") == -1) {
                        phase_one_allocation_failure
                    }
                }
                commas = 0;
                expect_comma = 0;      /* no comma is expected before data */
                while ((char_type = get_next_word(word, &word_ptr)) != -1
                       && word[0] != '\0') {
                    if (expect_comma != commas) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Improper comma use.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }

                    /* add data to the linked list */
                    data_tmp = get_data_int(word);

                    if (data_tmp == INVALID_INT) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Invalid data.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }

                    if (add_variable(variable_head,
                                     data_tmp & mask, *dc) == -1) {
                        phase_one_allocation_failure
                    } else (*dc)++;

                    /* check for commas */
                    expect_comma = 1;
                    commas = comma_checker(&word_ptr);
                    if (commas > 1) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Too many commas.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }
                }
                break;

            case STRING:
                if (label_flag == 1) {
                    label_flag = 0;
                    if (add_symbol(symbol_head, label_temp_ptr, *dc,
                                   "data") == -1) {
                        phase_one_allocation_failure
                    }
                }
                if (get_next_word(word, &word_ptr) != -1 && word[0] != '\0') {
                    if (word[0] == '"' && word[strlen(word) - 1] == '"') {
                        for (i = 1; i < strlen(word) - 1; i++) { /*add the string without the quotes*/
                            if (isprint(word[i]) == 0) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                "Invalid string character.\n",
                                        line_counter, filename);
                                error_flag = 1;
                                break;
                            } else if (add_variable(variable_head,
                                             get_ascii_value(word[i]),
                                             *dc) == -1) {
                                phase_one_allocation_failure
                            }
                            (*dc)++;
                        }
                        /* add null terminator */
                        if (add_variable(variable_head,
                                         get_ascii_value('\0'), *dc) == -1) {
                            phase_one_allocation_failure
                        }
                        (*dc)++;
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Cannot input more than 1 string.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            break;
                        }
                    } else {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Invalid string.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }
                } else {
                    fprintf(stdout, "Error: line %d in %s.\n       "
                                    "Missing string.\n",
                            line_counter, filename);
                    error_flag = 1;
                    break;
                }
                break;

            case EXTERN:
                commas = 0;
                expect_comma = 0;      /* no comma is expected before 1st extern */
                while (char_type != -1 /* if char_type was updated during the loop */
                       && (char_type = get_next_word(word, &word_ptr)) != -1 && word[0] != '\0') {
                    if (expect_comma != commas) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Improper comma use.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }

                    /* add extern to the  list */
                    if (add_symbol(symbol_head, word, INVALID_INT,
                                       "external") == -1) {
                            phase_one_allocation_failure
                    }

                    /* check for commas */
                    expect_comma = 1;
                    commas = comma_checker(&word_ptr);
                    if (commas > 1) {
                        fprintf(stdout, "Error: line %d in %s.\n       "
                                        "Too many commas.\n",
                                line_counter, filename);
                        error_flag = 1;
                        break;
                    }
                }
                break; /*end EXTERN case while*/

            case ENTRY:
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
                    if (add_symbol(symbol_head, label_temp_ptr, (*ic + 100),
                                                            "code") == -1){
                        phase_one_allocation_failure
                    }
                }
                /* initialize new command_t */
                if (init_command_word(command_head, &new_field) == -1) {
                    phase_one_allocation_failure
                }
                set_command_opcode(new_field, cmnd);
                switch (cmnd) {
                    /* two operands */
                    case 0: /*mov*/
                    case 1: /*cmp*/
                    case 2: /*add*/
                    case 3: /*sub*/
                    case 4: /*lea*/
                        /*first operand*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) {
                                set_addressing_method(word, new_field, 1);
                            } else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                word_type = FINISH;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        /*check for propper commas*/
                        if (comma_checker(&word_ptr) != 1) {
                            error_flag = 1;
                            if ((char_type = get_next_word(word, &word_ptr)) == -1) {
                                fprintf(stdout, "Error: line %d in %s.\n       "
                                                "Missing operand.\n",
                                    line_counter, filename);
                            }
                            else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Invalid comma use.\n",
                                    line_counter, filename);
                            }
                            word_type = FINISH;
                            break;
                        }

                        /*second oeprand*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) {
                                set_addressing_method(word, new_field, 2);
                            } else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                word_type = FINISH;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing operand.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        /* check for no extra operands */
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        break;
                        /*one operand*/
                    case 5:  /*clr*/
                    case 6:  /*not*/
                    case 7:  /*inc*/
                    case 8:  /*dec*/
                    case 9:  /*jmp*/
                    case 10: /*bne*/
                    case 11: /*red*/
                    case 12: /*prn*/
                    case 13: /*jsr*/
                        /*only destination operand*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                            operand_error = is_valid_operand(word, *macro_head);
                            if (operand_error == 1) /*valid*/
                                set_addressing_method(word, new_field, 2);
                            else {
                                PRINT_OPERAND_ERROR(operand_error);
                                error_flag = 1;
                                word_type = FINISH;
                                break;
                            }
                        } else {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Missing operand.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        /* check no extra operands: */
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        break;

                        /* no operands - handeled in a different area */
                    case 14: /*rts*/
                    case 15: /*stop*/
                        /*check no extra operands:*/
                        if ((char_type = get_next_word(word, &word_ptr)) != -1) {
                            fprintf(stdout, "Error: line %d in %s.\n       "
                                            "Too many operands.\n",
                                    line_counter, filename);
                            error_flag = 1;
                            word_type = FINISH;
                            break;
                        }
                        break;
                    default:
                        fprintf(stdout, "in COMMAND CASE: unknown error: line %d in %s.\n",
                                line_counter, filename);
                        error_flag = 1;
                        word_type = FINISH;
                        break;
                } /* end of cmnd switch */
                break;

            case ERROR:
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Invalid command.\n",
                        line_counter, filename);
                char_type = -1; /*finish this line*/
                error_flag = 1;
                break;

            case FINISH:
                break;
            
            case MISSING_CODE:
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Missing code after label.\n",
                                line_counter, filename);
                break;

            default:
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Unknown word type.\n",
                        line_counter, filename);
                error_flag = 1;
                break;
        } /* end of word_type switch */

        if (word_type == COMMAND) { /* reached end of line */
            new_field->l = calc_l(new_field, cmnd);
            (*ic) += new_field->l + 1;
            if (is_valid_addressing_method(new_field) == -1) {
                fprintf(stdout, "Error: line %d in %s.\n       "
                                "Operand type not allowed for this command.\n",
                                line_counter, filename);
                error_flag = 1;
            }
        }
    } /* end of line loop */
    update_ic(*symbol_head, *ic);

    if (*ic + *dc + 100 >= 4096) {
        fprintf(stdout, "Error: File %s.\n       "
                        "Code is too long, max memory is 4096 words.\n", filename);
        error_flag = 1;
    }

    if (error_flag == 1) {
        return -1;
    }

    return 0;
}

/**
 * @brief Creates a new command word and adds it to the linked list.
 * @param head the command list
 * @param ptr pointer to the newly created command word.
 * @return 0 on success, -1 on failure.
 */
int init_command_word(command_ptr *head, command_ptr *ptr) {
    command_ptr temp, new_node = (command_ptr) calloc(1, sizeof(command_t));

    if (new_node == NULL)
        return -1;

    new_node->are = 0x4;       /* 0b100 in binary, sets ARE to be 100 (only A) */
    new_node->dest_addr = 0x0; /* 0b0000 in binary */
    new_node->src_addr = 0x0;  /* 0b0000 in binary */
    new_node->opcode = 0x0;    /* 0b0000 in binary */
    new_node->l = 0x0;         /* 0b000 in binary */
    new_node->next = NULL;

    if (*head == NULL) { /* initialize the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
    *ptr = new_node;

    return 0;
}

/**
 * @brief Calculates the 'L' value (additional words in memory) for a command.
 * @param field the command_t struct
 * @param cmnd the command code
 * @return the value of 'l' based on the command
 */
int calc_l(command_ptr field, int cmnd) {
    if (cmnd == 14 || cmnd == 15)
        return 0; /*command without operands*/

    if (cmnd >= 5 && cmnd <= 13)
        return 1; /*command with 1 operand*/

    /*command with 2 operands, check if both are registers*/
    if ((field->src_addr == 0x4 || field->src_addr == 0x8) && (field->dest_addr == 0x4 || field->dest_addr == 0x8))
        return 1;
    return 2; /*last case, command with 2 operands*/
}

/**
 * @brief Sets the opcode of a command_t.
 * @param field Pointer to a command_t.
 * @param command The command afer is_valid_command, determine opcode from.
 * @return void
 */
void set_command_opcode(command_ptr field, int command) {
    if (command == 0)
        field->opcode = 0x0; /* mov */
    else if (command == 1)
        field->opcode = 0x1; /* cmp */
    else if (command == 2)
        field->opcode = 0x2; /* add */
    else if (command == 3)
        field->opcode = 0x3; /* sub */
    else if (command == 4)
        field->opcode = 0x4; /* lea */
    else if (command == 5)
        field->opcode = 0x5; /* clr */
    else if (command == 6)
        field->opcode = 0x6; /* not */
    else if (command == 7)
        field->opcode = 0x7; /* inc */
    else if (command == 8)
        field->opcode = 0x8; /* dec */
    else if (command == 9)
        field->opcode = 0x9; /* jmp */
    else if (command == 10)
        field->opcode = 0xA; /* bne */
    else if (command == 11)
        field->opcode = 0xB; /* red */
    else if (command == 12)
        field->opcode = 0xC; /* prn */
    else if (command == 13)
        field->opcode = 0xD; /* jsr */
    else if (command == 14)
        field->opcode = 0xE; /* rts */
    else if (command == 15)
        field->opcode = 0xF; /* stop */
}

/**
 * @brief Checks if the addressing method in the given command is valid.
 * 
 * This function takes a command pointer as input and checks if the addressing method 
 * specified in the command is valid based on the opcode and addressing modes.
 *
 * @param command pointer to the command structure to be checked.
 * @return 1 if the addressing method is valid, -1 otherwise.
 */
int is_valid_addressing_method(command_ptr command) {
    switch (command->opcode) {
        case 0x1: /* cmp */
        case 0xE: /* rts */
        case 0xF: /* stop */
            return 1;
        case 0x0: /* mov */
        case 0x2: /* add */
        case 0x3: /* sub */
            if (command->dest_addr == 0x1) {
                return -1;
            }
            return 1;
        case 0x4: /* lea */
            if (command->dest_addr == 0x1) {
                return -1;
            }
            if (command->src_addr != 0x2) {
                return -1;
            }
            return 1;
        case 0x5: /* clr */
        case 0x6: /* not */
        case 0x7: /* inc */
        case 0x8: /* dec */
        case 0xB: /* red */
            if (command->src_addr != 0x0) {
                return -1;
            }
            if (command->dest_addr == 0x1) {
                return -1;
            }
            return 1;
        case 0x9: /* jmp */
        case 0xA: /* bne */
        case 0xD: /* jsr */
            if(command->src_addr != 0x0){
                return -1;
            }
            if(command->dest_addr != 0x2 &&
               command->dest_addr != 0x4){
                return -1;
            }
            return 1;
        case 0xC: /* prn */
            if (command->src_addr != 0x0) {
                return -1;
            }
            return 1;
        default:
            return -1;
    }
}

/**
 * @brief Sets the addressing method in the command_t, used after is_valid_operand.
 * @param operand The operand to be parsed, NULL if it is a command without operands.
 * @param command Pointer to the command_t struct.
 * @param src_dest 1 if source, 2 if destination.
 * @return void
 */
void set_addressing_method(char *operand, command_ptr command, int src_dest) {
    if (src_dest == 1) { /* source operand */
        if (operand == NULL)
            command->src_addr = 0x0;

            /* Immediate addressing */
        else if (operand[0] == '#')
            command->src_addr = 0x1;

            /* Indirect register addressing */
        else if (operand[0] == '*')
            command->src_addr = 0x4; /*0b0100*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0
                 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            command->src_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else
            command->src_addr = 0x2; /*0b0010*/

    } else if (src_dest == 2) { /* destination operand */
        if (operand == NULL)
            command->dest_addr = 0x0;

            /* Immediate addressing */
        else if (operand[0] == '#')
            command->dest_addr = 0x1; /*0b0001*/

            /* Indirect register addressing */
        else if (operand[0] == '*')
            command->dest_addr = 0x4; /*0b0100*/

            /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0
                 && strlen(operand) == 2
                 && operand[1] >= '0' && operand[1] <= '7')
            command->dest_addr = 0x8; /*0b1000*/

            /* Direct addressing (label) */
        else
            command->dest_addr = 0x2; /*0b0010*/
    }
}

/**
 * @brief Checks if the given word is a valid operand.
 * @param word The word to be checked.
 * @param macro_head The table of macros.
 * @return 1 if the word is a valid operand, -1 command, -2 invalid immediate #,
 *         -3 invalid indirect register *, -4 macro, -5 invalid label 1st char,
 *         -6 invalid label
 */
int is_valid_operand(char *word, macro_ptr macro_head) {
    int i;

    if (is_valid_command(word) != -1) {
        return O_COMMAND; /*it is a command*/
    } else if (word[0] == '#') { /*needs to be a number constant*/
        /* Check for optional +- */
        i = 1;
        if (word[1] == '-' || word[1] == '+')
            i = 2;
        for (; word[i] != '\0'; i++)
            if (!isdigit(word[i])) {
                return O_IMMEDIATE;
            }
        if (i == 2 && !isdigit(word[1])) {
            return O_IMMEDIATE;
        }
    } else if (word[0] == '*') { /*needs to be a valid register*/
        if (!(strncmp(&word[1], "r", 1) == 0 && strlen(word) == 3
            && word[2] >= '0' && word[2] <= '7')) {
            return O_INDIRECT;
        }
    } else if (is_macro_name_valid(word, macro_head) == 2) {
        return O_MACRO;
    } else { /*needs to be a valid label*/
        if (isalpha(word[0]) == 0) {
            return O_START;
        }
        for (i = 0; word[i] != '\0'; i++) {
            if (!isalnum(word[i])) {
                return O_CHAR;
            }
        }
    }
    return O_VALID;
}

/**
 * @brief checks if the label name is valid
 * @param word the name of the label
 * @param symbol_head the head of the symbols table
 * @param macro_head the head of the macro table
 * @return -1 if command, -2 if label already exists, -3 if macro,
 *         -4 if register, -5 if doesn't start with a letter,
 *         -6 if isn't only letters and numbers, -7 if too long, 0 if valid
 */
int is_valid_label(char *word, symbol_ptr symbol_head,
                   macro_ptr macro_head) {
    int i = (int) strlen(word);
    char *registers[] = {"r0", "r1", "r2", "r3", "r4",
                         "r5", "r6", "r7"}; /* register names */
    symbol_ptr current = symbol_head;

    /* remove colon */
    if (word[i - 1] == ':')
        word[i - 1] = '\0';

    /* is it too long */
    if (i > MAX_LABEL_LENGTH)
        return L_LONG;

    /* is it a command */
    if (is_valid_command(word) != -1)
        return L_COMMAND;

    /* is it an existing label */
    while (current != NULL) {
        if (strcmp(word, current->name) == 0 && strcmp("entry", current->type) != 0) {
            return L_EXISTS;
        }
        current = current->next;
    }

    /* is it a macro */
    if (is_macro_name_valid(word, macro_head) == 2)
        return L_MACRO;

    /* is it a register */
    for (i = 0; i < 8; i++) {
        if (strcmp(word, registers[i]) == 0)
            return L_REGISTER;
    }

    /* does it start with a non-alpha character */
    if (isalpha(word[0]) == 0)
        return L_START;

    /* Check if label contains only alphanumeric characters */
    for (i = 0; word[i] != '\0'; i++) {
        if (!isalnum(word[i])) {
            return L_CHAR;
        }
    }

    return 0; /* valid */
}

/**
 * @brief Adds a new symbol to the symbol table or initializes if needed
 * @param head A pointer to the head of the symbol table linked list.
 * @param name The name of the symbol.
 * @param counter value of IC or DC.
 * @param type The type of the symbol: external / entry / data / code
 * @return 0 on success, -1 on failure.
 */
int add_symbol(symbol_ptr *head, char *name, int counter,
               char *type) {
    symbol_ptr temp, new_node = NULL; /* symbol nodes */

    new_node = (symbol_ptr) calloc(1, sizeof(symbol_t));
    if (new_node == NULL)
        return -1;

    as_strdup(&new_node->name, name);
    as_strdup(&new_node->type, type);

    new_node->counter = (strcmp(type, "external") == 0) ? INVALID_INT : counter; /* IC or DC */
    new_node->next = NULL;

    if (*head == NULL) { /* initializes the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }

    return 0;
}

/**
 * @brief Updates the counter of all data symbols with ic+100.
 * @param symbol_head A pointer to the symbol_head of the symbol linked list.
 * @param ic ic counter
 * @return void
 */
void update_ic(symbol_ptr symbol_head, int ic) {
    symbol_ptr temp = symbol_head;

    while (temp != NULL) {
        if (strcmp(temp->type, "data") == 0)
            temp->counter += ic + 100;
        temp = temp->next;
    }
}

/**
 * @brief Adds a new variable to the variable list or initializes it if needed.
 * @param head A pointer to the head of the variable linked list.
 * @param content The content of the variable.
 * @param counter DC counter
 * @return 0 on success, -1 on allocation failure.
 */
int add_variable(variable_ptr *head, int content, int counter) {
    variable_ptr temp, new_node = (variable_ptr) calloc(1, sizeof(variable_ptr));

    if (new_node == NULL)
        return -1;

    new_node->content = content;
    new_node->counter = counter; /*DC*/
    new_node->next = NULL;

    if (*head == NULL) { /* initialize the list */
        *head = new_node;
    } else {
        temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }

    return 0;
}

/**
 * @brief Parses a string and returns the integer value of the number
 *        it represents.
 * @param word A pointer to the first character of the string to be parsed.
 * @return The integer value of the number represented by the string.
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
    while (*word && !isspace(*word)) {
        if (!isdigit(*word))
            return INVALID_INT;
        result = result * 10 + (*word - '0');
        word++;
    }

    return sign * result;
}
