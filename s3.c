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

    char* outputFile = NULL;
    for(int i = 0; i < argsc; i++)
    {
        if(strcmp(args[i], ">") == 0 && i+1 < argsc) // ensure its not ls > without a file at the end
        {
            outputFile = args[i+1];
            args[i] = NULL; // for execvp
            break;
        }
    }

    if(outputFile == NULL)
    {
        // no output file
        printf("No output file specified\n");
        exit(1);
    }

    int fd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1)
    {
        printf("Error opening file\n");
        exit(1);
    }

    int dup2rc = dup2(fd, STDOUT_FILENO);
    if(dup2rc == -1)
    {
        printf("Error dup2 for output overwrite\n");
        exit(1);
    }

    close(fd);
    child(args, argsc);
}

void child_with_output_append(char *args[], int argsc)
{
    //  handles ">>" (append)
    //  check branch ezekiel
    char* outputFile = NULL;
    for(int i = 0; i < argsc; i++)
    {
        if(strcmp(args[i], ">>") == 0 && i+1 < argsc)
        {
            outputFile = args[i+1];
            args[i] = NULL; // for execvp
            break;
        }
    }

    if(outputFile == NULL)
    {
        // no output file
        printf("No output file specified\n");
        exit(1);
    }

    int fd = open(outputFile, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(fd == -1)
    {
        printf("Error opening file\n");
        exit(1);
    }

    int dup2rc = dup2(fd, STDOUT_FILENO);
    if(dup2rc == -1)
    {
        printf("Error dup2 for output append\n");
        exit(1);
    }

    close(fd);
    child(args, argsc);
}

void child_with_input_redirected(char *args[], int argsc)
{
    // handles "<" case (take input from file)
    char* inputFile = NULL;

    for(int i = 0; i < argsc; i++)
    {
        if(strcmp(args[i], "<") == 0 && i+1 < argsc)
        {
            // the file we are take it from
            inputFile = args[i+1];
            args[i] = NULL;
            break;
        }
    }

    if(inputFile == NULL)
    {
        printf("No input file specified\n");
        exit(1);
    }

    // we need to fd the read and read from input not terminal
    int fd = open(inputFile, O_RDONLY);

    if(fd == -1)
    {
        printf("Error opening input file\n");
        exit(1);
    }

    int dup2rc = dup2(fd, STDIN_FILENO);
    if(dup2rc == -1)
    {
        printf("Error with dup2 for input redirected\n");
        exit(1);
    }

    close(fd);

    child(args, argsc);
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
        else if(outputAppend)
        {
            child_with_output_append(args, argsc);
        }
        else if(outputOverwrite)
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