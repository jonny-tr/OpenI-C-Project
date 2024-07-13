#include "assembler.h"

/**
 * @brief duplicates a string
 * @param dest the destination string
 * @param s the string to duplicate
 * @return 0 if successful, -1 if not
 */
int as_strdup(char **dest, const char *s) {
    size_t size = strlen(s) + 1;
    char *temp = (char *)realloc(*dest, size * sizeof(char));

    if (temp == NULL) return -1;
    else *dest = temp;

    memcpy(*dest, s, size);

    return 0;
}

/**
 * @brief concatenates two strings
 * @param s1 the first string
 * @param s2 the second string
 * @return new string
 */
char *as_strcat(const char *s1, const char *s2) {
    const char *safe_s1 = s1 ? s1 : ""; /* if s1 is NULL, set it to "" */
    const char *safe_s2 = s2 ? s2 : ""; /* if s2 is NULL, set it to "" */
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
 * @return command's code if valid, -1 if it is not
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

    return -1;
}

int read_next_line(FILE *fd, char **line) {
    char buffer[81];

    while (fgets(buffer, 81, fd) != NULL
            && buffer[0] == ';'); /* skip comments */

    safe_free(*line)
    as_strdup(line, buffer);

    if (feof(fd)) {
        return -1;
    }

    return 0;
}
