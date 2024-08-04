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

        if ((phase_one(fd, ic, dc, symbols_list, variable_list, command_list, macro_table)) == -1) {
            fclose(fd);
            /*free_macro_table(macro_table);
            free_symbols_list(symbols_list);
            free_variables_list(variable_list);
            free_command_words_list(command_list);*/
            /*if (remove(tmp_file) != 0) {
                fprintf(stdout, "Error: Could not delete %s.\n", filename);
                fclose(fd);
            }shahar: we're not using this anymore, right? you can delete it*/
            continue;
        }

        /* TODO: need to create and send a command_list and a var_list */
        phase_two(fd, argv[i], symbols_list, variable_list, command_list,
                  ic, dc);

        cleanup:
        free_macro_table(macro_table);
        /* TODO: create these function:
        msg_from_shahar: I wrote them in phase_one.c, they are named a bit differently #sorrynotsorry */
        /*free_symbols_table(symbols_list);
        free_var_list(var_list);
        free_cmd_list(cmd_list);*/

        free_symbols_table(symbols_list);
        free_variable_list(variable_list);
        free_command_list(command_list);

        fclose(fd);
    }

    return 0;
}
