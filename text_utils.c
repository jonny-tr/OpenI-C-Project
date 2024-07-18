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

    temp = (char *)realloc(*dest, size * sizeof(char));
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
            return i;
        }
    }

    return -1;
}

/**
 * @brief Reads a line from the file and stores it in the given buffer.
 * @param file The file pointer to read from.
 * @param line The buffer to store the line.
 * @return 0 if successful, -1 otherwise.
 */
int read_next_line(FILE *file, char **line) {
    char buffer[81];

    while (fgets(buffer, 81, file) != NULL
           && buffer[0] == ';'); /* skip comments */

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';

    safe_free(*line)
    if (as_strdup(line, buffer) != 0) return -1;

    return 0;
}

/**
 * @brief checks if the next character is a comma
 * @param line line to check
 * @param position the position in the command
 * @return amount of commas, 0 if there are no commas
 */
int comma_checker(char *line, int *position) {
    int commas = 0; /* counter */

    /* Skip whitespaces */
    while (line[*position] == ' ' || line[*position] == '\t') {
        ++(*position);
    }

    while (line[*position] == ',') {
        ++(*position);
        ++commas;
    }

    return commas;
}

void get_word(char *position, char *word) { /*position is the line*/
    int i = 0;
    while (position[i] != ' ' && position[i] != ',' && position[i] != '\t' && position[i] != '\0') {
        word[i] = position[i];
        i++;
    }
    word[i] = '\0'; /* null terminate the word */
}


int get_next_word(char *line, char *word, char **word_ptr) {
    char *p = *word_ptr;
    char *w = word;

    /*Skip whitespaces*/
    while (*p == ' ' || *p == '\t' || *p == ',' || *p == '\0') {
        if (*p == '\0') {
            return -1; /*End of line*/
        }
        p++;
    }

    /*Get the word*/
    while (*p && *p != ' ' && *p != '\t' && *p != ',' && *p != '\0') {
        *w++ = *p++;
    }

    *w = '\0';

    /*Update the pointer*/
    *word_ptr = p;

    /*There is a word*/
    return 0;
}

int get_word_type(char *word) {
    /*comment for future shahar: 
    add check if there is a space before ":" to add the proper error message*/
    int i=0;
    
    if(strcmp(word,".data")==0) return DATA;
    if(strcmp(word,".string")==0) return STRING;
    if(strcmp(word,".entry")==0) return ENTRY;
    if(strcmp(word,".extern")==0) return EXTERN;
    if(position[i]=="."){
        fprintf(stdout, "Invalid command\n");
        return ERROR;
    }
    while (word[i] != '\0') i++;
    if(word[i]==":") return LABEL;
    /*if(is_valid_command(word)!=-1)*/ return COMMAND;
    /*return OPERAND;*/
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
