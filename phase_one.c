#include "assembler.h"

#define LINE_SIZE 81
#define MAX_LABEL_LENGTH 31


#define CHECK_UNEXPECTED_COMMA(char_type, error_flag) \
    if ((char_type) == 1) { \
        fprintf(stdout, "Error: Unexpected comma\n"); \
        (error_flag) = 1; \
        break; \
    }

/*@brief phase_one does the first pass on the file and builds the symbold and variables tables

@param fd the file after pre_assembler
@param symbol_table the symbol table
@param cmd_list_head the head of the command list
@param macro_table the macro table from pre_assembler
@param IC needs to be 0
@param DC needs to be 0

@return 0 on success, -1 on failure;
*/
int phase_one (FILE *fd, int IC, int DC, 
            symbols_ptr symbol_table, variable_ptr variable_table,
            command_ptr command_table, macro_ptr macro_table) {

    char line[LINE_SIZE], word[LINE_SIZE], label_temp[LINE_SIZE];
    char *word_ptr, *label_temp_ptr = label_temp;
    int label_flag = 0, error_flag=0, expect_comma ; /*1 = on, 0 = off*/
    int i, cmnd, word_type, data_tmp, commas;
    int char_type; /* -1 line end, 0 word, 1 comma */
    command_ptr new_field = (command_ptr) malloc(sizeof(command_word));
    if (new_field == NULL) {allocation_failure};
    while (read_next_line(fd, (char **)&line) != -1) {
        word_ptr = line;
        while ((char_type = get_next_word(line, word, &word_ptr)) != -1) {
            CHECK_UNEXPECTED_COMMA(char_type, error_flag);
            word_type = get_word_type(word);
            switch (word_type) {
                case LABEL: /* TODO: I think errors need to include the file they were found in and the line number -yoni */
                    switch (is_valid_label(word, symbol_table, macro_table)) {
                        case -1:
                            fprintf(stdout, "Cannot use a command as a label\n");
                            error_flag = 1;
                            break;
                        case -2:
                            fprintf(stdout, "Label already exists\n");
                            error_flag = 1;
                            break;
                        case -3:
                            fprintf(stdout, "Cannot use a macro as a label\n");
                            error_flag = 1;
                            break;
                        case -4:
                            fprintf(stdout, "Cannot use a register as a label\n");
                            error_flag = 1;
                            break;
                        case -5:
                            fprintf(stdout, "Label must start with a letter\n");
                            error_flag = 1;
                            break;
                        case -6:
                            fprintf(stdout, "Label must only contain letters and numbers\n");
                            error_flag = 1;
                            break;
                        case -7:
                            fprintf(stdout, "Label is too long, max length is %d\n", MAX_LABEL_LENGTH);
                            error_flag = 1;
                            break;
                        case 0: /*valid label*/
                            if(label_flag==1) {
                                fprintf(stdout, "Cannot use two labels at once");
                                error_flag = 1;
                            }
                            else{
                                label_flag = 1;
                                as_strdup(&label_temp_ptr, word);
                            }
                            break;
                    } /*end label switch*/

                case DATA:
                    if (label_flag == 1) {
                        label_flag = 0;
                        if(add_symbol(&symbol_table, label_temp, DC, "data")==-1) {allocation_failure};
                    }
                    expect_comma = 0;
                    while ((char_type=get_next_word(line, word, &word_ptr)) != -1 && word[0] != '\0') {
                        if(expect_comma==1){
                            commas=comma_checker(line, &word_ptr);
                            if(commas==0){
                                fprintf(stdout, "Missing comma\n");
                                error_flag = 1;
                                break;
                            }
                            else if(commas>1){
                                fprintf(stdout, "Too many commas\n");
                                error_flag = 1;
                                break;
                            }
                            else expect_comma=0;
                        }
                        data_tmp = get_data_int(word);
                        if (data_tmp == INVALID_INT) {
                            fprintf(stdout, "Invalid data\n");
                            error_flag = 1;
                            break;
                        }
                        if(add_variable(&variable_table, twos_complement(data_tmp), DC)==-1) {allocation_failure};
                        DC++;
                        expect_comma = 1;
                    }
                    break;

                case STRING:
                    if (label_flag == 1) {
                        label_flag = 0;
                        if(add_symbol(&symbol_table, label_temp, DC, "data")==-1) {allocation_failure};
                    }
                    if (get_next_word(line, word, &word_ptr) != -1 && word[0] != '\0') {
                        if (word[0] == '"' && word[strlen(word) - 1] == '"')
                            for (i = 1; i < strlen(word) - 1; i++) { /*add the string without the quotes*/
                                if(add_variable(&variable_table, get_ascii_value(word[i]), DC)==-1) {allocation_failure};
                                DC++;
                            }
                        else {
                            fprintf(stdout, "Invalid string\n");
                            error_flag = 1;
                            break;
                        }
                    }
                    break;
                case EXTERN:
                    expect_comma = 0;
                    while (get_next_word(line, word, &word_ptr) != -1 && word[0] != '\0') {
                        if(expect_comma==1){
                            commas=comma_checker(line, &word_ptr);
                            if(commas==0){
                                fprintf(stdout, "Missing comma\n");
                                error_flag = 1;
                                break;
                            }
                            else if(commas>1){
                                fprintf(stdout, "Too many commas\n");
                                error_flag = 1;
                                break;
                            }
                            else expect_comma=0;
                        }
                        if(add_symbol(&symbol_table, word, INVALID_INT, "external")==-1) {allocation_failure};
                        /*continue adding allocation faliur for add_symbol and then for add_variable
                        write free symbols and variable*/
                        expect_comma = 1;
                    }
                    break;
                case ENTRY:
                    if(get_next_word(line, word, &word_ptr)==0){
                        if(is_valid_label(word, symbol_table, macro_table)==0){
                            if(add_symbol(&symbol_table, word, IC + 100, "code")==-1) {allocation_failure};
                            IC++;
                        }
                        else {
                            fprintf(stdout, "Invalid label for Entry\n");
                            error_flag = 1;
                        }
                    } else {
                        fprintf(stdout, "Missing label for Entry\n");
                        error_flag = 1;
                    }
                    break;

                case COMMAND:
                    cmnd=is_valid_command(word);
                    if (cmnd == -1) {
                        fprintf(stdout, "%s: invalid command\n", word);
                        error_flag = 1;
                        break;
                    }
                    if (label_flag == 1) {
                        label_flag = 0;
                        if(add_symbol(&symbol_table, label_temp, (IC + 100), "code")==-1) {allocation_failure};
                        IC++;
                    }
                    /*initialize new command_word*/
                    if(init_command_word(&command_table, &new_field)==-1) {allocation_failure};
                    set_command_opcode(new_field, cmnd);
                    switch (cmnd) {
                            /*two operands*/
                            case 0: /*mov*/
                            case 1: /*cmp*/
                            case 2: /*add*/
                            case 3: /*sub*/
                            case 4: /*lea*/
                                /*first operand*/
                                if((char_type=get_next_word(line, word, &word_ptr)) != -1){
                                    CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                                    if (is_valid_operand(word, macro_table))
                                        set_addressing_method(word, new_field, 1);
                                    else {
                                        error_flag = 1;
                                        break;
                                    }
                                }
                                else{
                                    fprintf(stdout, "Missing operands\n");
                                    error_flag = 1;
                                    break;
                                }
                                /*check for propper commas*/
                                if(comma_checker(line, &word_ptr) != 1){
                                    error_flag = 1;
                                    fprintf(stdout, "Invalid comma use\n");
                                    break;
                                }
                                
                                /*second oeprand*/
                                if((char_type=get_next_word(line, word, &word_ptr)) != -1){
                                    if (is_valid_operand(word, macro_table))
                                        set_addressing_method(word, new_field, 2);
                                    else{
                                        error_flag = 1;
                                        break;
                                    }
                                }
                                else {
                                    fprintf(stdout, "Missing destination operand\n");
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
                                if((char_type=get_next_word(line, word, &word_ptr)) != -1){
                                    CHECK_UNEXPECTED_COMMA(char_type, error_flag);
                                    if (is_valid_operand(word, macro_table))
                                        set_addressing_method(word, new_field, 2);
                                    else {
                                        error_flag = 1;
                                        break;
                                    }
                                }
                                else {
                                    fprintf(stdout, "Missing operand\n");
                                    error_flag = 1;
                                    break;
                                }
                                break;

                            /*no operands - already taken care of*/    
                            case 14: /*rts*/
                            case 15: /*stop*/
                                break;
                        }
                    new_field->l = calc_l(new_field, cmnd);
                    IC += new_field->l;
            } /*end of switch*/
        } /*end of line while*/
        /***label_flag = 0;?***/
    }
    end_phase_one_update_counter(&symbol_table, IC);
    if(error_flag == 1) return -1;
    return 0;
}

/**
 * Initializes a new command word and adds it to a linked list.
 *
 * @param head A pointer to the head of the linked list.
 * @param ptr A pointer to the newly created command word.
 *
 * @return 0 on success, -1 on failure.
 */
int init_command_word(command_ptr *head, command_ptr *ptr) {
    command_ptr new_node = (command_ptr) malloc(sizeof(command_word));
    if (new_node == NULL) {
        fprintf(stderr, "Memory allocation for new command_word failed\n");
        return -1;
    }

    new_node->are = 0x4; /* 0b100 in binary *//* automatically sets ARE to be 100 (only A) */
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
 * Calculates the value of 'L' for a given command.
 *
 * @param field pointer to the command_word struct
 * @param cmnd the command code
 *
 * @return the value of 'l' based on the command
 */
int calc_l(command_word *field, int cmnd){
    if(cmnd==14 || cmnd==15) return 0; /*command without operands*/
    else if(cmnd>=5 && cmnd<=13) return 1; /*command with one operand*/

    /*commands with two operands*/
    /*check if both operands are registers: 0100 or 1000*/
    else if ((field->src_addr == 0x4 || field->src_addr == 0x8) && /*0b0100||0b1000*/
         (field->dest_addr == 0x4 || field->dest_addr == 0x8)) { /*0b0100||0b1000*/
            return 1;
        }
        else return 2;
    
}

/**
 * Sets the opcode of a command_word.
 *
 * @param field Pointer to a command_word.
 * @param command The command afer is_valid_command, determine opcode from.
 * @return void
 *
 */
void set_command_opcode(command_word *field, int command) {
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
 * Sets the addressing method in the command_word, used after is_valid_operand.
 *
 * @param operand The operand to be parsed, NULL if it is a command without operands.
 * @param field Pointer to the command_word struct.
 * @param src_dest 1 if source, 2 if destination .
 */
void set_addressing_method(char *operand, command_word *field, int src_dest) {
    if (src_dest == 1) { /* source operand */
        if (operand == NULL) field->src_addr = 0x0;

        /* Immediate addressing */
        else if (operand[0] == '#') field->src_addr = 0x1;    

        /* Indirect register addressing */
        else if (operand[0] == '*') field->src_addr = 0x4; /*0b0100*/

        /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7')
            field->src_addr = 0x8; /*0b1000*/

        /* Direct addressing (label) */
        else field->src_addr =0x2; /*0b0010*/

    } else if (src_dest == 2) { /* destination operand */
        if (operand == NULL) field->dest_addr = 0x0;    

        /* Immediate addressing */
        else if (operand[0] == '#') field->dest_addr = 0x1; /*0b0001*/

        /* Indirect register addressing */
        else if (operand[0] == '*') field->dest_addr = 0x1;
           
        /* Direct register addressing */
        else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7')
            field->dest_addr = 0x8; /*0b1000*/

        /* Direct addressing (label) */
        else field->dest_addr = 0x2; /*0b0010*/
    }
}

/**
 * Frees the memory allocated for the linked list of command words.
 *
 * @param head A pointer to the head of the linked list.
 * @return 0 if the function ran successfully, 1 if the head is NULL.
 */
/*yoni you wrote this in cleanup yeah?- shahar 30/07/24*/
/*int free_command_words_list(command_ptr *head) {
    command_ptr current, next;

    if (*head == NULL) return 1; /*nothing to free*/

  /*  current = *head;
    while (current != NULL) {
        next = current->next;
        safe_free(current);
        current = next;
    }
    *head = NULL;
    return 0;
}*/


/**
 * Checks if the given word is a valid operand.
 *
 * @param word The word to be checked.
 * @return 1 if the word is a valid operand, 0 otherwise.
 *
 * @throws None.
 */
int is_valid_operand(char *word, macro_ptr macro_table) {
    int i;
    if(is_valid_command(word)!=-1) {
        fprintf(stdout, "A command cannot be used as an operand");
        return 0; /*it is a command*/
    }
    else if(word[0]=='#'){ /*needs to be a number constant*/
    /* Check for optional +- */
        i=1;
        if (word[1] == '-' || word[1] == '+') i = 2;
        for (; word[i] != '\0'; i++)
            if (!isdigit(word[i])) {
                fprintf(stdout, "Invalid immediate operand, it after # must be a number\n");
                return 0;
            }
        if(i==2 && !isdigit(word[2])){
            fprintf(stdout, "Invalid immediate operand, it after # must be a number\n");
            return 0;
        }
    }

    else if (word[0] == '*'){ /*needs to be a valid register*/
        if (!(strncmp(word, "r", 2) == 0 && strlen(word) == 3 && word[2] >= '0' && word[2] <= '7')) {
            fprintf(stdout, "%s is not a valid register\n", word);
            return 0;
        }
    }
    else if (is_macro_name_valid(word, macro_table) == 2) {
        fprintf(stdout, "%s is a macro, invalid operand\n", word);
        return 0;
    }
    else { /*needs to be a valid label*/
        if (isalpha(word[0]) == 0) {
            fprintf(stdout, "%s is not a valid label, must start with a letter\n", word);
            return 0;
        }
        for (i = 0; word[i] != '\0'; i++) {
            if (!isalnum(word[i])) {
                fprintf(stdout, "%s is not a valid label, must only contain letters and numbers\n", word);
                return 0;
            }
        }
    }
    return 1;
}

/**
 * @brief checks if the label name is valid
 * @param name the name of the label
 * uses the macro and the symbols tables, needs access
 * @return -1 if command, -2 if label already exists, -3 if macro, -4 if register,
 *  -5 if doesn't start with a letter, -6 if isn't only letters and numbers, -7 if too long,
 *  0 if valid
 */
int is_valid_label(char *word, symbols_ptr symbols_table_head, macro_ptr macro_table_head) {
    int i = 0;
    char *used_registers[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7" }; /* register names */
    symbols_list *current = symbols_table_head;

    /*remove colon*/
    while (word[i] != '\0') {
        if (word[i] == ':') {
            word[i] = '\0';
        }
        i++;
    }
    /*is it too long*/
    if(i>MAX_LABEL_LENGTH) return -7;

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
 * Adds a new symbol to the symbol table or initializes if needed
 *
 * @param head A pointer to the head of the symbol table linked list.
 * @param name The name of the symbol.
 * @param counter IC or DC.
 * @param type The type of the symbol: external / entry / data / code 
 * @return 0 on success, -1 on failure.
 *
 */
int add_symbol(symbols_ptr *head, char *name, int counter, char *type) {
    symbols_ptr new_node = (symbols_ptr) malloc(sizeof(symbols_list));
    if (new_node == NULL) {
        fprintf(stdout, "Memory allocation for new symbol failed\n");
        return -1;
    }
    as_strdup(&new_node->name, name);
    as_strdup(&new_node->type, type);
    new_node->counter = (strcmp(type, "external") == 0) ? INVALID_INT : counter; /*IC or DC*/
    new_node->next = NULL;

    if (*head == NULL) { /*initialize the list*/
        *head = new_node;
    } else {
        symbols_ptr temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_node;
    }
    return 0;
}

/**
 * Frees the memory allocated for the linked list of symbols.
 *
 * @param head A pointer to the head of the linked list.
 * @return 0 if the function ran successfully, 1 if the head is NULL.
 */
/*yoni you wrote this in cleanup yeah?- shahar 30/07/24*/
/*int free_symbols_list(symbols_ptr *head) {
    symbols_ptr current, next;
    if (*head == NULL) return 1; /*nothing to free*/

 /*   current = *head;
    while (current != NULL) {
        next = current->next;
        safe_free(current->name);
        safe_free(current->type);
        safe_free(current);
        current = next;
    }
    *head = NULL;

    return 0;
}*/

/**
 * Updates the counter of all data symbols with IC+100.
 *
 * @param head A pointer to the head of the symbol linked list.
 * @param IC IC
 */
void end_phase_one_update_counter(symbols_ptr *head, int IC) {
    symbols_ptr temp = *head;
    while (temp != NULL) {
        if (strcmp(temp->type, "data") == 0) temp->counter += IC + 100;
        temp = temp->next;
    }
}

/**
 * Adds a new variable to the variable linked list or initializes if needed.
 *
 * @param head A pointer to the head of the variable linked list.
 * @param content The content of the variable.
 * @param counter DC
 * @return 0 on success, -1 on failure.
 *
 */
int add_variable(variable_t **head, int content, int counter) {
    variable_ptr new_node = (variable_ptr) malloc(sizeof(variable_ptr));
    if (new_node == NULL) {
        fprintf(stdout, "Memory allocation for new variable failed\n");
        return -1;
    }
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

/*yoni you wrote this in cleanup yeah?- shahar 30/07/24*/
/*int free_variables_list(variable_ptr *head) {
    variable_ptr current, next;
    if (*head == NULL) return 1; /*nothing to free*/

/*    current = *head;
    while (current != NULL) {
        next = current->next;
        safe_free(current->content);
        safe_free(current);
        current = next;
    }
    *head = NULL;

    return 0;
}*/


/**
 * Parses a string and returns the integer value of the number it represents.
 *
 * @param word A pointer to the first character of the string to be parsed.
 * @return The integer value of the number represented by the string.
 *
 * @throws None.
 */
int get_data_int(char *word) {
    int result = 0;
    int sign = 1;

    /*Check for sign*/
    if (*word == '-') {
        sign = -1;
        word++;
    } else if (*word == '+') {
        word++;
    }

    /*read the number*/
    while (*word && *word != ' ' && *word != '\t' && *word != ',' && *word != '\0') {
        if (!isdigit(*word)) {
            fprintf(stdout, "Not a number\n");
            return INVALID_INT;
        }
        result = result * 10 + (*word - '0');
        word++;
    }
    return sign * result;
}
