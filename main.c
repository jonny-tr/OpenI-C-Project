#include "assembler.h"

/**
 * @brief Checks if any files were passed as arguments and starts the program.
 * @param argc The number of arguments passed to the program.
 * @param argv The arguments passed to the program.
 * @return 0 if the program ran successfully, 1 if an error occurred.
 */
int main(int argc, char *argv[]) {
    unsigned int i; /* counter */
    FILE *fd; /* file pointer */
    char *filename; /* filename */

    if (argc < 2) {
        fprintf(stdout, "Error: No files specified.\n");
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (pre_assembler(&argv[i]) == 1) continue;
        filename = assembler_strcat(argv[i], ".am");
        fd = fopen(filename, "r");
        if (fd == NULL) continue; /* skip */
        fclose(fd);
    }

    return 0;
}
