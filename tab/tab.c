#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <termios.h>

#define MAX_LINE 1024

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

char **get_file_completions(const char *prefix, int *count) {
    DIR *dir = opendir(".");
    struct dirent *entry;
    char **matches = malloc(100 * sizeof(char*));
    *count = 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
            matches[*count] = strdup(entry->d_name);
            (*count)++;
        }
    }
    closedir(dir);
    return matches;
}

void handle_tab_completion(char *line, int *pos) {
    int count;
    char **matches = get_file_completions(line, &count);
    
    if (count == 1) {
        strcpy(line, matches[0]);
        *pos = strlen(line);
        printf("\rmyshell> %s", line);
        fflush(stdout);
    } else if (count > 1) {
        printf("\n");
        for (int i = 0; i < count; i++) {
            printf("%s  ", matches[i]);
        }
        printf("\nmyshell> %s", line);
        fflush(stdout);
    }
    
    for (int i = 0; i < count; i++) free(matches[i]);
    free(matches);
}

void handle_backspace(char *line, int *pos) {
    if (*pos > 0) {
        line[--(*pos)] = '\0';
        printf("\b \b");
        fflush(stdout);
    }
}

void handle_regular_char(char c, char *line, int *pos) {
    if (*pos < MAX_LINE - 1) {
        line[(*pos)++] = c;
        putchar(c);
        fflush(stdout);
    }
}

void process_input(char *line, int *pos) {
    char c;
    
    while (1) {
        read(STDIN_FILENO, &c, 1);
        
        if (c == '\t') {  // TAB key
            handle_tab_completion(line, pos);
        } else if (c == '\n') { // enter key
            printf("\n");
            break;
        } else if (c == 127) {  // Backspace
            handle_backspace(line, pos);
        } else {
            handle_regular_char(c, line, pos);
        }
    }
}

int main() {
    enable_raw_mode();
    
    char line[MAX_LINE] = {0};
    int pos = 0;
    
    printf("myshell> ");
    fflush(stdout);
    
    process_input(line, &pos);
    
    printf("You entered: %s\n", line);
    return 0;
}