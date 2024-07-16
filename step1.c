#include "assembler.h"

#define LINE_SIZE 81

/*move to their funtions*/
char line[LINE_SIZE], word[LINE_SIZE], label_temp[LINE_SIZE];
int label_flag=0; /*1 = label, 0=no label*/
int IC=0, DC=0; /* instruction counter and data counter*/
int len; /*line length*/
int word_type;

/*@brief

@param fd

@return DC
*/
int step1(char **fd){
    while ((len = read_next_line(fd, &line)) != -1 || !feof(fd)){
        while (len > 0){
            get_word(&line, &word);
            len -= strlen(word);
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
                while(word[0] != '\0'){
                    get_word(&line, &word);
                    add_variable(&variable_table, get_data_int(word), DC);
                    DC++;
                    /*add an error if between two variables there is a space and not a comma*/
                }
                break;
                
            case STRING:
                if(label_flag == 1){
                    label_flag = 0;
                    add_symbol(symbol_table, label_temp, DC, "data");    
                }
            case COMMAND:
                if(label_flag == 1){
                    label_flag = 0;
                    add_symbol(symbol_table, label_temp, (IC+100), "code");    
                }

            default:
                break;
            }
        }
    }
    if (next_word[strlen(next_word) - 1] == ':')
        continue;
    if ((strcmp(next_word, ".data") == 0) || (strcmp(next_word, ".string") == 0) || (strcmp(next_word, ".extern") == 0))
        continue;

    if (strcmp(next_word, ".entry") == 0)
    {
        while (temp_flag == 0)
        {
            next_check if (strcmp(next_word, ".end") == 0)
            {
                temp_flag = 1;
                continue;
            }
            if (entry_update(symbol_table, next_word) == -1)
            {
                error_flag = 1;
                break;
            }
        }
        continue;
    }

/*before sending a variable, if it is int convernt to string*/

/*if label and then variable, add label, go through variables and add them until end of line*/
/*if there is a lable and a variable, need to make sure DC is only incremented after adding
to both lists*/
return DC;
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