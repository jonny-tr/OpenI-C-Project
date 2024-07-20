#include "assembler.h"

/**
 * @brief Checks if any files were passed as arguments and starts the program
 * @param argc The number of arguments passed to the program
 * @param argv The arguments passed to the program
 * @return 0 if the program ran successfully, 1 if an error occurred
 */
int main(int argc, char *argv[]) {
    int i, dc = 30, ic = 100; /* counters */
    /* TODO: remove dc and ic initialaizers */
    FILE *fd; /* file pointer */
    char *filename, *tmp_file = NULL; /* filename */
    macro_ptr macro_table = NULL; /* macro table */
    symbols_ptr symbols_list = NULL;

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

        /*
        if ((dc = phase_one(fd, filename, macro_table)) == -1) {
            goto cleanup;
        }
        */

        /* TODO: need to create and send a command_list and a var_list */
        phase_two(fd, argv[i], symbols_list, variable_head, cmd_list, ic, dc);

        cleanup:
        free_macro_table(macro_table);
        /* TODO: create these function: */
        free_symbols_table(symbols_list);
        free_variable_list(var_list);
        free_command_list(cmd_list);

        fclose(fd);
    }

    return 0;
}
