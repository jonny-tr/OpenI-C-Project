#include "assembler.h"

/**
 * @brief frees the macro table
 * @param macro_table_head the table of macros
 * @return 0 if the function ran successfully, 1 if macro_table_head is NULL
 */
int free_macro_table(macro_ptr macro_table_head) {
    macro_ptr current, next;
    str_node_ptr current_content, *next_content;

    if ((current = macro_table_head) == NULL) return 1;

    while (current != NULL) {
        next = current->next;
        safe_free(current->name)
        current_content = current->content_head;
        while (current_content != NULL) {
            next_content = &current_content->next;
            safe_free(current_content->str)
            safe_free(current_content)
            current_content = *next_content;
        }
        safe_free(current)
        current = next;
    }

    return 0;
}

/**
 * @brief frees the symbol table
 * @param symbols_list_head the symbol table
 * @return 0 if the function ran successfully, 1 if symbols_list_head is NULL
 */
int free_symbols_table(symbols_ptr symbols_list_head) {
    symbols_ptr current, next;

    if ((current = symbols_list_head) == NULL) return 1;

    while (current != NULL) {
        next = current->next;
        safe_free(current)
        current = next;
    }

    return 0;
}

/**
 * @brief frees the variable list
 * @param var_list_head the variable list
 * @return 0 if the function ran successfully, 1 if var_list_head is NULL
 */
int free_variable_list(variable_ptr var_list_head) {
    variable_ptr current, next;

    if ((current = var_list_head) == NULL) return 1;

    while (current != NULL) {
        next = current->next;
        safe_free(current)
        current = next;
    }

    return 0;
}

/**
 * @brief frees the command list
 * @param cmd_list_head the command list
 * @return 0 if the function ran successfully, 1 if cmd_list_head is NULL
 */
int free_command_list(command_ptr cmd_list_head) {
    command_ptr current, next;

    if ((current = cmd_list_head) == NULL) return 1;

    while (current != NULL) {
        next = current->next;
        safe_free(current)
        current = next;
    }

    return 0;
}
/**
 * @brief Frees all the memory allocated for the given data structures.
 *
 * @param macro_table_head The head of the macro table.
 * @param symbols_list_head The head of the symbols list.
 * @param var_list_head The head of the variable list.
 * @param cmd_list_head The head of the command list.
 * 
 * should it return something?
 */
void free_all(macro_ptr macro_table_head, symbols_ptr symbols_list_head, \
            variable_ptr var_list_head, command_ptr cmd_list_head){
    free_macro_table(macro_table_head);
    free_symbols_table(symbols_list_head);
    free_variable_list(var_list_head);
    free_command_list(cmd_list_head);
}
