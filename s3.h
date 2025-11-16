#ifndef _S3_H_
#define _S3_H_

///See reference for what these libraries provide
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

///Constants for array sizes, defined for clarity and code readability
#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256

///Enum for readable argument indices (use where required)
enum ArgIndex
{
    ARG_PROGNAME,
    ARG_1,
    ARG_2,
    ARG_3,
};

///With inline functions, the compiler replaces the function call 
///with the actual function code;
///inline improves speed and readability; meant for short functions (a few lines).
///the static here avoids linker errors from multiple definitions (needed with inline).
static inline void reap()
{
    wait(NULL);
}

///Shell I/O and related functions (add more as appropriate)
void read_command_line(char line[], char lwd[]);
void construct_shell_prompt(char shell_prompt[], char lwd[]);
void parse_command(char line[], char *args[], int *argsc);

bool command_with_redirection(char line[]);

bool is_cd(char line[]);
void init_lwd(char lwd[]);

bool command_with_pipes(char line[]);
void parse_pipes(char line[], char *commands[], int *commandCount);

bool command_with_batch(char line[]);
void parse_batch(char line[], char *batched_commands[], int *batchedCommandCount);

///Child functions (add more as appropriate)
void child(char *args[], int argsc);
void child_with_output_overwrite(char *args[], int argsc, char lwd[]);
void child_with_output_append(char *args[], int argsc, char lwd[]);
void child_with_input_redirected(char *args[], int argsc, char lwd[]);

///Program launching functions (add more as appropriate)
void launch_program(char *args[], int argsc);
void launch_program_with_redirection(char *args[], int argsc, char lwd[]);
void run_cd(char *args[], int argsc, char lwd[]);
void launch_pipes(char *commands[], int commandCount, char lwd[]);
void launch_batch(char *batched_commands[], int batchedCommandCount, char lwd[]);

// misc helper funcs
void trimWhitespace(char **str_ptr);

// new main
void resolve(char line[], char lwd[]);

// all subshell specific funcs
bool command_with_subshell(char line[]);
int parse_what_is_in_subshell(int startIdx, char line[], char *subshell_content[]);
void launch_subshell(char *subshell_command, char* lwd);
void launch_subshell_with_redirection(char *subshell_command, char *redirect_cmd, char *lwd);
void resolve_command_with_subshell(char line[], char *lwd);
char* extract_subshell_content(char *subshell_command);

#endif