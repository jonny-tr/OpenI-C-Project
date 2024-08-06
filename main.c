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
    FILE *fd; /* file pointer */
    char *filename; /* filename */
    macro_ptr macro_table = NULL; /* macro table */
    symbols_ptr symbols_list = NULL; /* symbol list */
    variable_ptr variable_list = NULL; /* variable list */
    command_ptr command_list = NULL; /* command list */

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
            goto cleanup;
        }

        fd = fopen(filename, "r");
        if (fd == NULL) {
            fprintf(stdout, "Error: Could not open file %s.\n", filename);
            goto cleanup; /* skip */
        }

        if ((phase_one(fd, argv[i], ic, dc, &symbols_list, variable_list,
                       command_list, macro_table)) == -1) {
            goto cleanup;
        }

        phase_two(fd, argv[i], symbols_list, variable_list, command_list,
                  *ic, *dc);

        cleanup:
        free_macro_table(macro_table);
        free_symbols_table(symbols_list);
        free_variable_list(variable_list);
        free_command_list(command_list);

        fclose(fd);
    }

    return 0;
}
