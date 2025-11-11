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

void child_with_output_overwrite(char *file_name)
{
    //  handles this ">" (overwrite)
    //  open the output file and redirect stdout to that file
    int fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("open failed");
        exit(1);
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        exit(1);
    }
    close(fd);
}

void child_with_output_append(char *file_name)
{
    //  handles ">>" (append)
    int fd = (open(file_name, O_WRONLY | O_CREAT | O_APPEND, 0644));
    if (fd == -1)
    {
        perror("open failed");
        exit(1);
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        perror("dup2 failed");
        exit(1);
    }
    close(fd);

}

void child_with_input_redirected(char *file_name)
{
    // handles "<" (take input from file)
    int fd = (open(file_name, O_RDONLY, 0644));
    if (fd == -1)
    {
        perror("open failed");
        exit(1);
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        perror("dup2 failed");
        exit(1);
    }
    close(fd);
}

//launches programs with redirection
void launch_program_with_redirection(char *args[], int argsc)
{

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
        //  child process
        printf("Entering redirection child process\n");
        
        char *redirection_type = NULL;
        char *file_name = NULL;
        int redirection_index = -1;

        //  for loop to look for the redirection symbols and file name
        for(int i = 0; i < argsc; i++)
        {
            if(strcmp(args[i], ">>") == 0)
            {
                redirection_type = ">>";
            }
             if(strcmp(args[i], ">") == 0)
            {
                redirection_type = ">";
            }
            if(strcmp(args[i], "<") == 0)
            {
                redirection_type = "<";
            }
            if (redirection_type != NULL)
            {  
                if (i + 1 < argsc)
                {
                    file_name = args[i+1];
                    redirection_index = i;
                    break;
                }
                else
                {
                    printf("Missing filename after redirection symbol");
                    exit(1);
                }
            }
        }
        // call appropriate redirection function to redirect I/O based on the symbol found
        if (redirection_type != NULL)
        {
            if(strcmp(redirection_type,">>")==0)
            {
                child_with_output_append(file_name);
            }
            if(strcmp(redirection_type, ">")==0)
            {
                child_with_output_overwrite(file_name);
            }
            if(strcmp(redirection_type,"<")==0)
            {
                child_with_input_redirected(file_name);
            }
        }
        
        // terminate args before the redirection symbol
        args[redirection_index] = NULL; 
        
        char *programmeName = args[0];
        
        // execute the command with redirected I/O
        if(execvp(programmeName, args) == -1)
        {
            printf("child function failed\n");
            exit(1);
        }
    }
    else
    {
        wait(NULL);
        printf("Parent process now (redirection) ...\n");
    }
}