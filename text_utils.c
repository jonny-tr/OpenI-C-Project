#include "assembler.h"

/**
 * @brief duplicates a string
 * @param s a string
 * @return new string
 */
char *assembler_strdup(const char *s) {
    char *new_str = calloc(strlen(s) + 1, sizeof(char));

    if (new_str == NULL) return NULL;

    strcpy(new_str, s);
    strcat(new_str, "\0");

    return new_str;
}

/**
 * @brief concatenates two strings
 * @param s1 the first string
 * @param s2 the second string
 * @return new string
 */
char *assembler_strcat(const char *s1, const char *s2) {
    const char *safe_s1 = s1 ? s1 : "";
    const char *safe_s2 = s2 ? s2 : "";
    char *new_str = calloc(strlen(safe_s1) + strlen(safe_s2) + 1,
                           sizeof(char));

    if (new_str == NULL) return NULL;
    strcpy(new_str, safe_s1);
    strcat(new_str, safe_s2);

    return new_str;
}

/**
 * @brief checks if the command is valid
 * @param command the command
 * @return command's code if valid, 1 if it is not
 */
int is_valid_command(char *command) {
    char *valid[] = {".data", ".string", ".entry", ".extern",
                     "mov", "cmp", "add", "sub", "lea",
                     "clr", "not", "inc", "dec", "jmp",
                     "bne", "red", "prn", "jsr", "rts",
                     "stop"}; /* valid commands */
    int i; /* counter */

    for (i = 0; i < 20; ++i) {
        if (strcmp(command, valid[i]) == 0) {
            return i;
        }
    }

    return 0;
}
