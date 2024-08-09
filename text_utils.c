#include "assembler.h"


/**
 * @brief duplicates a string
 * @param dest the destination string
 * @param s the string to duplicate
 * @return 0 if successful, -1 if not
 */
int as_strdup(char **dest, const char *s) {
    size_t size = (s != NULL) ? strlen(s) + 1 : 1;
    char *temp = NULL;

    temp = (char *) realloc(*dest, size * sizeof(char));
    if (temp == NULL) return -1;

    *dest = temp;

    if (s != NULL) memcpy(*dest, s, size);
    else (*dest)[0] = '\0';

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

    for (i = 0; i < 20; i++) {
        if (strcmp(command, valid[i]) == 0) {
            return i-4;
        }
    }

    return -1;
}

/**
 * @brief Reads a line from the file and stores it in the given buffer.
 * @param file The file pointer to read from.
 * @param line The buffer to store the line.
 * @return 0 if successful, -1 if an EOF reached.
 */
int read_next_line(FILE *file, char *line) {
    char buffer[81];

    while (fgets(buffer, 81, file) != NULL
           && buffer[0] == ';'); /* skip comments */

    if (feof(file)) return -1;

    memcpy(line, buffer, strlen(buffer) + 1);

    return 0;
}

/**
 * @brief the function reads the next part of the line
 * @param line the line to read from
 * @param position the position in the line
 * @param next_part the next part of the line
 * @return 0 if successful, 1 if line finished,
 *          -1 if an error occurred
 */
int read_next_word(const char *line, int *position, char **next_part) {
    char c; /* strings */
    int buffer = 0; /* counter */

    if (line[*position] == '\n') return -1;

    while (isspace(line[*position])) ++(*position); /* skip whitespaces */

    if (line[*position] == ',') return 1;

    while (isspace(c = line[*position]) != 0
           && c != ',' && c != ':') {
        *next_part[buffer] = c;
        ++buffer;
        ++(*position);
    }
    *next_part[buffer] = '\0';
    printf("%s\n", *next_part);

    return 0;
}

/**
 * @brief checks if the next character is a comma
 * @param line line to check
 * @param position the position in the command
 * @return amount of commas, 0 if there are no commas
 */

int comma_checker(char *line, char **word_ptr) {
    char *p = *word_ptr;
    int commas = 0;

    /* Count commas, including those separated by whitespaces */
    while (*p == ',' || isspace(*p)) {
        if (*p == ',') commas++;
        p++;
    }
    *word_ptr = p;
    return commas;
}

/**
 * Get the next word from a line of text.
 *
 * @param line The line of text to search for the next word.
 * @param word A pointer to a character array where the word will be stored.
 * @param word_ptr A pointer to a pointer to the current position in the line.
 *
 * @return 0 if a word was found, -1 if end of the line was reached,
 *         1 if comma before word
 */

int old_get_next_word(char *line, char *word, char **word_ptr) {
    char *p = *word_ptr, *w = word;

    /* Skip whitespaces */
    while (isspace(*p)) {
        if (*p == '\n' || *p =='\0') {
            return -1; /* End of line */
        }
        p++;
    }

    /* Check for a comma */
    if (*p == ',') {
        *word_ptr = p;
        return 1;
    }

    /* Get the word */
    while (*p && !isspace(*p) && *p != ',' && *p != ':') {
        *w++ = *p++;
    }

    /* Handle the case where the word ends with a colon */
    if (*p == ':') {
        *w++ = *p++;
    }

    *w = '\0';
    *word_ptr = p;
    /*fprintf(stdout, "debugging: extracted word is: %s\n", word);*/
    return 0;
}

int get_next_word(char *line, char *word, char **word_ptr) {
    char *p = *word_ptr, *w = word;

    /** Skip leading whitespaces **/
    while (isspace(*p)) {
        if (*p == '\n' || *p == '\0') {
            *word_ptr = p; /** Update word_ptr to the end of the line **/
            return -1; /** End of line **/
        }
        p++;
    }

    /** Handle the case where the line is completely empty or just has whitespaces **/
    if (*p == '\0') {
        *word_ptr = p; /** Update word_ptr to the end of the line **/
        return -1; /** End of line **/
    }

    /** Check for a comma **/
    if (*p == ',') {
        *word_ptr = p + 1; /** Move past the comma **/
        return 1; /** Indicate that a comma was found **/
    }

    /** Extract the word **/
    while (*p && !isspace(*p) && *p != ',' && *p != ':') {
        *w++ = *p++;
    }

    /** Handle the case where the word ends with a colon **/
    if (*p == ':') {
        *w++ = *p++;
    }

    *w = '\0'; /** Null-terminate the word **/
    *word_ptr = p; /** Update word_ptr to the new position **/
    return 0; /** Indicate a word was found **/
}





/**
 * @brief returns the type of the word
 * @param word the word to check
 * @return the type of the word (data, string, label, etc.)
 *         or -1 if an error occurred
 */
int get_word_type(char *word) {
    int i = 0;

    if (strcmp(word, ".data") == 0) return DATA;
    if (strcmp(word, ".string") == 0) return STRING;
    if (strcmp(word, ".entry") == 0) return ENTRY;
    if (strcmp(word, ".extern") == 0) return EXTERN;
    if (word[i] == '.') return ERROR; /* starts with . but not a saved word */

    while (word[i] != '\0') i++;
    if (i >= 2 && word[i - 1] == ':') {
        if (isspace(word[i - 2])) return -2;
        return LABEL;
    }

    if (is_valid_command(word) != -1) return COMMAND;
    fprintf(stdout, "debugging: ERROR, word is %s\n", word);

    return ERROR;
}

/**
 * @brief returns the command as a number
 * @param cmd the command
 * @return the command as a number
 */
int command_to_num(command_word cmd) {
    int full_bits = (cmd.opcode << 11) | (cmd.src_addr << 7) |
                    (cmd.dest_addr << 3) | cmd.are;

    return full_bits;
}

/**
 * @brief the function converts a number to twos complement
 * @param num a number to convert
 * @return the converted number
 */
int twos_complement(int num) {
    int mask = 0x7FFF; /* 15 bits mask */

    return (~num + 1) | (~mask);
}

/**
 * @brief returns the ASCII value of a character.
 * @param ch The character to convert.
 * @return The ASCII value.
 */
int get_ascii_value(char ch) {
    return (int) ch;
}
