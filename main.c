#include "assembler.h"

/**
 * @brief Checks if any files were passed as arguments and starts the program
 * @param argc The number of arguments passed to the program
 * @param argv The arguments passed to the program
 * @return 0 if the program ran successfully, 1 if an error occurred
 */
int main(int argc, char *argv[]) {
    int i, instruction_counter = 0, data_counter = 0; /* counter */
    int *ic = &instruction_counter, *dc = &data_counter; /* counters */
    FILE *am_fd; /* file pointer */
    char *am_filename; /* filename */
    macro_ptr macro_head = NULL; /* macro table */
    symbol_ptr symbol_head = NULL; /* symbol table */
    variable_ptr variable_head = NULL; /* variable table */
    command_ptr command_head = NULL; /* command table */

    if (argc < 2) {
        fprintf(stdout, "Error: No files specified.\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        am_filename = as_strcat(argv[i], ".am");
        
        if (pre_assembler(&argv[i], macro_head) == -1) {
            goto cleanup; /* skip */
        }

        am_fd = fopen(am_filename, "r");
        if (am_fd == NULL) {
            fprintf(stdout, "Error: Could not open file %s.\n", am_filename);
            goto cleanup; /* skip */
        }

        if ((phase_one(am_fd, am_filename, ic, dc, &symbol_head, &variable_head,
                       &command_head, &macro_head)) == -1) {
            goto cleanup;
        }

        rewind(am_fd);
        phase_two(am_fd, argv[i], symbol_head, variable_head, command_head,
                *ic, *dc);

        cleanup:
        free_macro_table(macro_head);
        free_symbols_table(symbol_head);
        free_variable_list(variable_head);
        free_command_list(command_head);

        if (am_fd) fclose(am_fd);
    }

    return 0;
}
