#include "assembler.h"

enum word_type_e{ /*word type enum*/
    ERROR=-1,
    LABEL=0,
    DATA=1,
    STRING=2,
    ENTRY=3,
    EXTERN=4,
    COMMAND=5   
};
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

    for (i = 0; i < 20; ++i) {
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
 * @param max_length The maximum length of the line buffer.
 * @return The length of the line read, excluding the newline character. If the end of file is reached or only comments are found, returns 0.
 *
 */
    int read_next_line(FILE *file, char *line, int max_length) {
        int len = 0;
        while (fgets(line, max_length, file) != NULL) {
            /*Remove newline character if present*/
            len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                len--;
            }

            /*Check if the line is not a comment*/
            if (line[0] != ';') {
                return len;
            }
        }
        return 0;  /*Return 0 if end of file is reached or only comments are found*/
    }

/**
 * @brief checks if the next character is a comma
 * @param line line to check
 * @param position the position in the command
 * @return amount of commas, 0 if there is no comma
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
int get_word_type(char *position) {
    /*comment for future shahar: 
    this function should be called after get_word,
    it needs to be changed to itterate over the word and not the line
    add check if there is a space before ":" to add the proper error message*/
    char *word;
    enum word_type_e word_type;
    int i=0;
        while (isspace(position[i])){
            i++;
        } /*skip whitespace*/
        
    
    if(position[i]=="."){
        word_type= DATA; /*.data or .string or .entry or .extern*/
    }
    while (position[i] != ' ' && position[i] != ',' && position[i] != '\t' && position[i] != '\0') {
        word[i] = position[i];
        i++;
    }
    word[i] = '\0';
    if(word_type==DATA){
        if(strcmp(word,".data")==0) return DATA;
        if(strcmp(word,".string")==0) return STRING;
        if(strcmp(word,".entry")==0) return ENTRY;
        if(strcmp(word,".extern")==0) return EXTERN;
        /*case: invalid command, error message*/
    }
    if(word[i]==":") return 0;
    if(is_valid_command(word)!=-1) return COMMAND;
    return ERROR;
}

