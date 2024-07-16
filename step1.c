#include "assembler.h"

#define LINE_SIZE 81

/*function declarations*/
int is_valid_label(char *word);
void remove_colon(char *label);

char line[LINE_SIZE]; /*buffer*/
char word[LINE_SIZE];
char label_temp[LINE_SIZE];
int label_flag=0; /*1 = label, 0=no label*/
int IC=0, DC=0; /* instruction counter and data counter*/
int len; /*line length*/

/*@brief

@param fd

@return DC
*/
int step1(char **fd) {
    while(len = read_next_line(fd, *line, LINE_SIZE)!=0){
        /*before is_valid_label, use get_word_type*/
    };

    return DC;
}

/**
 * @brief checks if the label name is valid
 * @param name the name of the label
 * switches the label flag to 1 if valid
 * uses the macro and the symbols tables, needs access
 * @return -1 if command, -2 if label already exists, -3 if macro, -4 if register, 0 if valid
 */
int is_valid_label(char *word, symbols_ptr symbols_table_head, macro_ptr macro_table_head,){
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
 * @param counter IC.
 * @param type The type of the symbol: external / entry / data
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
    new_node->counter = (strcmp(type, "external") == 0) ? NULL : counter + 100; /*IC+100*/
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
