#include "s3.h"

///Simple for now, but will be expanded in a following section
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
    char *token = strtok(line, " "); // "hello world\0" --> "hello\0" "world\0"
    // char* name = "Brandon" 
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
    ///Implement this function:

    ///Use execvp to load the binary 
    ///of the command specified in args[ARG_PROGNAME].
    ///For reference, see the code in lecture 3.
    char *programmeName = args[0];

    if(execvp(programmeName, args) == -1)
    {
        printf("child function failed\n");
        exit(1);
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
        printf("Entering child process\n");
        child(args, argsc);
    }
    else
    {
        wait(NULL);
        printf("Parent process now...\n");
    }
    return;
}

bool command_with_redirection(char line[])
{
    // '>' redirects standard output into a file
    // '<' takes input from file instead of keyboard
    if(strchr(line, '>') != NULL || strchr(line, '<') != NULL)
    {
        return true;
    }

    return false;
}

void child_with_output_overwrite(char *args[], int argsc)
{
    //  handles this ">" (overwrite)

    
}

void child_with_output_append(char *args[], int argsc)
{
    //  handles ">>" (append)

}

void child_with_input_redirected(char *args[], int argsc)
{
    // handles "<" case (take input from file)
}

void launch_program_with_redirection(char *args[], int argsc)
{
    // example of what we have to parse further
    // sort txt/phrases.txt > txt/phrases_sorted.txt

    // args[2] tell us whether output redirected ('>') or input redirected ('<)


    // how would i handle exit > hello.txt?
    if(strcmp(args[0], "exit") == 0) // args[0] == "exit"
    {
        printf("Exiting shell\n");
        exit(0);
    }

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
        // testing my commits
        printf("Entering redirection child process\n");
        
        // for loop to check if > or < 
        bool outputOverwrite = false;
        bool outputAppend = false;
        bool inputRedirect = false;


        for(int i = 0; i < argsc; i++)
        {
            if(strcmp(args[i], ">") == 0)
            {
                outputOverwrite = true;
                break;
            }
            if(strcmp(args[i], "<") == 0)
            {
                inputRedirect = true;
                break;
            }
            if(strcmp(args[i], ">>") == 0)
            {
                outputAppend = true;
                break;
            }
        }

        if(inputRedirect)
        {
            child_with_input_redirected(args, argsc);
        }
        if(outputAppend)
        {
            child_with_output_append(args, argsc);
        }
        if(outputOverwrite)
        {
            child_with_output_overwrite(args, argsc);
        }
    }
    else
    {
        wait(NULL);
        printf("Parent process now (redirection)...\n");
    }
    return;
}