#include "assembler.h"

#define LINE_SIZE 81

/*move to their funtions*/
char line[LINE_SIZE], word[LINE_SIZE], label_temp[LINE_SIZE];
char * word_ptr, * string_ptr;
int label_flag=0; /*1 = label, 0=no label*/
int IC=0, DC=0; /* instruction counter and data counter*/
int len; /*line length, do i need it?*/
int i;
int word_type;

/*@brief

@param fd

@return DC
*/
int step1(char **fd){ /*do i still need len?*/
    while ((len = read_next_line(fd, &line)) != -1 || !feof(fd)){
        word_ptr=line;
        while (get_next_word(&line, &word, &word_ptr) != -1){
            /*len -= strlen(word);*/
            word_type = get_word_type(&word);
            switch (word_type){
            case LABEL:
                switch (is_valid_label(word, *symbol_table, *macro_table)){
                case -1:
                    fprintf(stdout, "Cannot use a command as a label\n");
                    break;
                case -2:
                    fprintf(stdout, "Label already exists\n");
                    break;
                case -3:
                    fprintf(stdout, "Cannot use a macro as a label\n");
                    break;
                case -4:
                    fprintf(stdout, "Cannot use a register as a label\n");
                    break;
                case 0: /*valid*/
                    label_flag = 1;
                    strdup(label_temp, word);
                    break;
                }
            
            case DATA:
                if(label_flag == 1){
                    label_flag = 0;
                    add_symbol(symbol_table, label_temp, DC, "data");    
                }
                while(get_next_word(&line, &word, &word_ptr) != -1 && word[0] != '\0'){
                    /*need to check if operands are not commands, labels etc*/
                    add_variable(&variable_table, get_data_int(word), DC);
                    DC++;
                    /*add an error if between two variables there is a space and not a comma?*/
                }
                break;
                
            case STRING:
                if(label_flag == 1){
                    label_flag = 0;
                    add_symbol(symbol_table, label_temp, DC, "data");    
                }
                if(get_next_word(&line, &word, &word_ptr) != -1 && word[0] != '\0'){
                    if(word[0]=='"' && word[strlen(word)-1]=='"')
                        for(i=1; i<strlen(word-1); i++){ /*add the string without the quotes*/
                            add_variable(&variable_table, word[i], DC);
                            DC++;
                        }
                    else {
                        fprintf(stdout, "Invalid string\n");
                        break;
                    }   
                }
                break;
            case EXTERN:
                while(get_next_word(&line, &word, &word_ptr) != -1 && word[0] != '\0'){
                    add_symbol(symbol_table, word, NULL, "external");
                }
                break;
            case ENTRY:
                if(is_valid_label(get_next_word(&line, &word, &word_ptr), *symbol_table, *macro_table) == 0){
                    add_symbol(symbol_table, word, IC+100, "code");
                    IC++;
                }
                else fprintf(stdout, "Invalid label for Entry\n");
                break;

            case COMMAND:
                if(is_valid_command(word) == -1){
                    fprintf(stdout, "%s: invalid command\n", word);
                    break;
                }
                if(label_flag == 1){
                    label_flag = 0;
                    add_symbol(symbol_table, label_temp, (IC+100), "code"); 
                    IC++;   
                }



            default:
                break;
            }
        } /*end of line while*/
        label_flag=0;

    }
    
    

/*before sending a variable, if it is int convernt to string*/

/*if label and then variable, add label, go through variables and add them until end of line*/
/*if there is a lable and a variable, need to make sure DC is only incremented after adding
to both lists*/
return DC;
}

int command_table(int cmnd){
    switch (cmnd) {
        case 0: /*mov*/
        case 1: /*cmp*/
        case 2: /*add*/
        case 3: /*sub*/
        case 4: /*lea*/
        case 5: /*clr*/
        case 6: /*not*/
        case 7: /*inc*/
        case 8: /*dec*/
        case 9: /*jmp*/
        case 10: /*bne*/
        case 11: /*red*/
        case 12: /*prn*/
        case 13: /*jsr*/
        case 14: /*rts*/
        case 15: /*stop*/
        default:
    }

}

int calc_l(command_word *field, int cmnd){
    if(cmnd==14 || cmnd==15) return 1; /*command without operands*/
    else if(cmnd>=5 && cmnd<=13) return 2; /*command with one operand*/

    /*commands with two operands*/
    /*check if both operands are registers: 0100 or 1000*/
    else if ((field->src_addr == 0b0100 || field->src_addr == 0b1000) && 
            (field->dest_addr == 0b0100 || field->dest_addr == 0b1000)) {
            return 2;
        } else return 3;

    
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
    if (command == 0) field->opcode = 0b0000;       /*mov*/
    else if (command == 1) field->opcode = 0b0001;  /*cmp*/
    else if (command == 2) field->opcode = 0b0010;  /*add*/
    else if (command == 3) field->opcode = 0b0011;  /*sub*/
    else if (command == 4) field->opcode = 0b0100;  /*lea*/
    else if (command == 5) field->opcode = 0b0101;  /*clr*/
    else if (command == 6) field->opcode = 0b0110;  /*not*/
    else if (command == 7) field->opcode = 0b0111;  /*inc*/
    else if (command == 8) field->opcode = 0b1000;  /*dec*/
    else if (command == 9) field->opcode = 0b1001;  /*jmp*/
    else if (command == 10) field->opcode = 0b1010; /*bne*/
    else if (command == 11) field->opcode = 0b1011; /*red*/
    else if (command == 12) field->opcode = 0b1100; /*prn*/
    else if (command == 13) field->opcode = 0b1101; /*jsr*/
    else if (command == 14) field->opcode = 0b1110; /*rts*/
    else if (command == 15) field->opcode = 0b1111; /*stop*/
   /* else field->opcode = 0; // Invalid command, set to 0 */
}

/**
 * Parses the operand and sets the addressing method and ARE fields in the command_word.
 *
 * @param operand The operand to be parsed.
 * @param field Pointer to the command_word struct.
 * @param src_dest 1 if source, 2 if destination .
 */
void set_addressing_method_and_are (char *operand, command_word *field, int src_dest) {
    if(src_dest==1){ /*source operand*/

        /* Immediate addressing*/
        if (operand[0] == '#') {
            field->src_addr = 0b0001;
            field->are = 0b100;
        }

        /*Indirect register addressing */
        else if (operand[0] == '*') {
            if(!(strncmp(operand, "r", 2) == 0 && strlen(operand) == 3 && operand[2] >= '0' && operand[2] <= '7')) {
                fprintf(stdout, "%s is not a valid register\n", operand);
            }
            field->src_addr = 0b0100;
            field->are = 0b100; 
        }
        
        /* Direct register addressing */
         else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7') {
            field->src_addr = 0b1000;
            field->are = 0b100;
        } 
        
        /*Direct addressing (label)*/
        else {            
            field->src_addr = 0b0010;
            *are = 0b001; /*depends if label is extern or not*/
        }
    }
    else if(src_dest==2){ /*destination operand*/

        /* Immediate addressing*/
        if (operand[0] == '#') {
            field->dest_addr = 0b0001;
            field->are = 0b100;
        }

        /*Indirect register addressing */
        else if (operand[0] == '*') {
            if(!(strncmp(operand, "r", 2) == 0 && strlen(operand) == 3 && operand[2] >= '0' && operand[2] <= '7')) {
                fprintf(stdout, "%s is not a valid register\n", operand);
            }
            field->dest_addr = 0b0100;
            field->are = 0b100; 
        }
        
        /* Direct register addressing */
         else if (strncmp(operand, "r", 1) == 0 && strlen(operand) == 2 && operand[1] >= '0' && operand[1] <= '7') {
            field->dest_addr = 0b1000;
            field->are = 0b100;
        } 
        
        /*Direct addressing (label)*/
        else {
            field->dest_addr = 0b0010;
            *are = 0b001; /*depends if label is extern or not*/
        }
    }
}

/**
 * @brief checks if the label name is valid
 * @param name the name of the label
 * switches the label flag to 1 if valid
 * uses the macro and the symbols tables, needs access
 * @return -1 if command, -2 if label already exists, -3 if macro, -4 if register, 0 if valid
 */
int is_valid_label(char *word, symbols_ptr symbols_table_head, macro_ptr macro_table_head){
    remove_colon(*word);
    /*is it a command*/
    if(is_valid_command(word) != -1) return -1;

    /*is it an existing label*/
    symbols_list *current = symbols_table_head;
    while (current != NULL) {
        if (strcmp(name, current->name) == 0) {
            return -2;
        }
        current = current->next;
    }

    /*is it a macro*/
    if(is_macro_name_valid(*word, macro_table_head) == 2) return -3;

    /*is it a register*/
    char *register[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"}; /* register names */
    int i;
    for (i = 0; i < 8; i++) {
        if (strcmp(word, register[i]) == 0) return -4;
    }
    
    label_flag =1;
    return 0; /*valid*/
}

void remove_colon(char *label){
    int i = 0;
    while(label[i] != '\0'){
        if(label[i] == ':'){
            label[i] = '\0';
        }
        i++;
    }
}
/*counter is IC*/

/**
 * Adds a new symbol to the symbol table or initializes if needed
 *
 * @param head A pointer to the head of the symbol table linked list.
 * @param name The name of the symbol.
 * @param counter IC or DC.
 * @param type The type of the symbol: external / entry / data / code 
 *
 */
void add_symbol(symbols_ptr *head, char *name, int counter, char *type) {
    symbols_ptr new_node = (symbols_ptr)malloc(sizeof(symbols_list));
    if (new_node == NULL) {
        fprintf(stdout, "Memory allocation for new symbol failed\n");
        return;
    }
    new_node->name = strdup(name);
    new_node->type = strdup(type);
    new_node->counter = (strcmp(type, "external") == 0) ? NULL : counter; /*IC or DC*/
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
}

/**
 * Adds a new variable to the variable linked list or initializes if needed.
 *
 * @param head A pointer to the head of the variable linked list.
 * @param content The content of the variable.
 * @param counter DC
 * @return void
 *
 */
void add_variable(variable_t *head, char *content, int counter) {
    variable_ptr new_node = (variable_ptr)malloc(sizeof(variable_ptr));
    if (new_node == NULL) {
        fprintf(stdout, "Memory allocation for new variable failed\n");
        return;
    }
    new_node->content = strdup(content);
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
}

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
            return NULL;
        }
        result = result * 10 + (*word - '0');
        word++;
    }
    return sign * result;
}