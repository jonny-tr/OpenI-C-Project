#include "assembler.h"

/* TODO pre_assembler needs to run on the passed files by adding a .as suffix
 * it will create a .am file after spreading macros
 * a macro start with macr and ends with end_macr */
int pre_assembler(int argc, char **argv) {
    unsigned int i; /* counter */
    FILE *fd; /* file pointer */

    for (i = 1; i < argc; i++) {
        /* open the file */
        argv[i] = strcat(argv[i], ".am");
        fd = fopen(argv[i], "r");
        if (fd == NULL) {
            fprintf(stderr, "Error: Could not open file %s\n", argv[i]);
            return 1;
        }

        /* close the file */
        fclose(fd);
    }

    return 0;
}
