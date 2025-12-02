#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <dirent.h> // open close and read directories
#include <glob.h>

#define MAX_LINE 1024
#define MAX_ARGS 128
#define MAX_PROMPT_LEN 256
bool contains_wildcard_in_args(char *args[], int argsc);
int expand_globs_in_args(char *args[], int argsc, char *expanded_args[]);

static inline void reap()
{
    wait(NULL);
}

void construct_shell_prompt(char shell_prompt[])
{
    strcpy(shell_prompt, "[s3]$ ");
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt);
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)
    {
        perror("fgets failed");
        exit(1);
    }
    ///Remove newline (enter)
    line[strlen(line) - 1] = '\0';
}

void parse_command(char line[], char *args[], int *argsc)
{
    ///Implements simple tokenization (space delimited)
    ///Note: strtok puts '\0' (null) characters within the existing storage, 
    ///to split it into logical cstrings.
    ///There is no dynamic allocation.

    ///See the man page of strtok(...)
    char *token = strtok(line, " ");
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL; ///args must be null terminated
}


///Launch related functions
void child(char *args[], int argsc)
{
    // include globbing functionality
    /*
    char *expanded_args[MAX_ARGS];
    int expanded_count = expand_globs_in_args(args, argsc, expanded_args);
    */

    if(contains_wildcard_in_args(args, argsc))
    {
        char *expanded_args[MAX_ARGS];
        int expanded_count = expand_globs_in_args(args, argsc, expanded_args);

        if(expanded_count > 0)
        {
            if(execvp(expanded_args[0], expanded_args) == -1)
            {
                printf("Failed to execute globbed command\n");
                exit(1);
            }
        }
        else
        {
            printf("No arguments after glob expansion\n");
            exit(1);
        }
    }
    else
    {
        if (execvp(args[0], args) == -1)
        {
            printf("Command execution failed\n");
            exit(1);
        }
    }
}

void launch_program(char *args[], int argsc)
{
    ///Implement this function:
    ///Handle the 'exit' command;
    ///so that the shell, not the child process,
    ///exits.
    if(strcmp(args[0], "exit") == 0) // args[0] == "exit"
    {
        printf("Exiting shell\n");
        exit(0);
    }

    ///fork() a child process.
    ///In the child part of the code,
    ///call child(args, argv)
    ///For reference, see the code in lecture 2.
    int rc = fork();
    if(rc < 0)
    {
        // fork failed
        printf("fork failed\n");
        exit(1);
    }
    else if(rc == 0)
    {
        // child (new process)
        //printf("Entering child process\n\n");
        //printf("\n");
        child(args, argsc);
    }
    else
    {
        wait(NULL);
        //printf("\nParent process now...\n");
        //printf("\n");
    }
    return;
}

bool contains_wildcard_in_args(char *args[], int argsc)
{
    for(int i = 0; i < argsc; i++)
    {
        if(strchr(args[i], '*') != NULL)
        {
            return true;
        }
    }

    return false;
}

// Expand glob patterns and return new argument list
int expand_globs_in_args(char *args[], int argsc, char *expanded_args[])
{
    int expanded_count = 0;
    glob_t glob_result;
    
    for (int i = 0; i < argsc; i++) {
        // Check if this argument has a wildcard
        if (strchr(args[i], '*') != NULL) {
            // Expand the pattern
            int ret = glob(args[i], 0, NULL, &glob_result);
            
            if (ret == 0) {
                // Add all expanded matches
                for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                    if (expanded_count >= MAX_ARGS - 1) {
                        printf("Too many arguments after glob expansion\n");
                        break;
                    }
                    expanded_args[expanded_count++] = strdup(glob_result.gl_pathv[j]);
                }
                globfree(&glob_result);
            } else if (ret == GLOB_NOMATCH) {
                // No matches found, keep the original pattern
                printf("No matches found for: %s\n", args[i]);
                if (expanded_count < MAX_ARGS - 1) {
                    expanded_args[expanded_count++] = args[i];
                }
            } else {
                // Other glob error
                printf("Glob error for pattern: %s\n", args[i]);
                if (expanded_count < MAX_ARGS - 1) {
                    expanded_args[expanded_count++] = args[i];
                }
            }
        } else {
            // No wildcard, just copy the argument
            if (expanded_count < MAX_ARGS - 1) {
                expanded_args[expanded_count++] = args[i];
            }
        }
    }
    
    expanded_args[expanded_count] = NULL;
    return expanded_count;
}

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;

    while (1) {
        read_command_line(line);

        parse_command(line, args, &argsc);

        launch_program(args, argsc); 

        reap();
    }

    return 0;
    
}