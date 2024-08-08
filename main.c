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
    char *filename; /* filename */
    macro_ptr macro_table = NULL; /* macro table */
    symbols_ptr symbol_table = NULL; /* symbol list */
    variable_ptr variable_table = NULL; /* variable list */
    command_ptr command_table = NULL; /* command list */

    if (argc < 2) {
        fprintf(stdout, "Error: No files specified.\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        filename = as_strcat(argv[i], ".am");
        
        if (pre_assembler(&argv[i], macro_table) == -1) {
            if (remove(filename) != 0) {
                fprintf(stdout, "Error: Could not delete %s.as.\n", argv[i]);
            }
            goto cleanup; /* skip */
        }

        am_fd = fopen(filename, "r");
        if (am_fd == NULL) {
            fprintf(stdout, "Error: Could not open file %s.\n", filename);
            goto cleanup; /* skip */
        }

        if ((phase_one(am_fd, filename, ic, dc, &symbol_table, variable_table,
                       command_table, macro_table)) == -1) {
            goto cleanup;
        }

        phase_two(am_fd, filename, symbol_table, variable_table, command_table,
                  *ic, *dc);

        cleanup:
        free_macro_table(macro_table);
        free_symbols_table(symbol_table);
        free_variable_list(variable_table);
        free_command_list(command_table);

        fclose(am_fd);
    }

    return 0;
}
