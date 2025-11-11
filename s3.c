#include "s3.h"

///Simple for now, but will be expanded in a following section
void construct_shell_prompt(char shell_prompt[], char lwd[])
{
    char cwd[MAX_PROMPT_LEN-6];
    if(getcwd(cwd, sizeof(cwd)) == NULL)
    {
        strcpy(shell_prompt, "[s3]$ ");
    }
    else
    {
        snprintf(shell_prompt, MAX_PROMPT_LEN, "[%s]$ ", cwd);
    }
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];
    construct_shell_prompt(shell_prompt, lwd);
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

    /*for(int i = 0; i < argsc; i++)
    {
        printf("child arg[%d] : %s\n", i, args[i]);
    }
    printf("\n");*/

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
        printf("Entering child process\n\n");
        child(args, argsc);
    }
    else
    {
        wait(NULL);
        printf("\nParent process now...\n");
    }
    return;
}

bool command_with_redirection(char line[])
{
    // '>' redirects standard output into a file
    // '<' takes input from file instead of keyboard
    // ">>" gets catched in > we go down specifics later
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
            if(strcmp(args[i], ">>") == 0)
            {
                outputAppend = true;
                break;
            }
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

void init_lwd(char lwd[])
{
    if(getcwd(lwd, MAX_PROMPT_LEN-6) == NULL)
    {
        printf("getcwd in init_lwd failed\n");
        exit(1);
    }
}

bool is_cd(char line[])
{
    // parse the cd out and ensure its just cd not something like cdabc

    if(strncmp(line, "cd", 2) == 0)
    {
        // cd .... --> check if right after cd its a space or end of string
        if(line[2] == ' ' || line[2] == '\0')
        {
            return true;
        }
    }

    return false;
}

void run_cd(char *args[], int argsc, char lwd[])
{
    /*
    To implement cd correctly, it must be treated as a special case within your shell and 
    executed in the main process using the chdir() system call.

    As always, look up the man page for chdir to see how it works. 
    In short, the parameters one expects to pass to cd are passed to chdir—namely, 
    the directory to which we want to change. 
    This includes the special . and .. directories, 
    which can be passed directly to the chdir system call as parameter.
    */

    char *path;
    char currDir[MAX_PROMPT_LEN-6];

    if(getcwd(currDir, sizeof(currDir)) == NULL)
    {
        printf("getcwd in run_cd failed\n");
        exit(1);
    }

    // ensure its max argcs 2
    if(argsc > 2)
    {
        printf("Too many arguments for cd\n");
        exit(1);
    }

    // cd no args
    // cd is used without any arguments, it should change the directory to the user's home
    if(argsc == 1)
    {
        path = getenv("HOME");
        if(path == NULL)
        {
            printf("cd: HOME not set\n");
            exit(1);
        }
    }
    // When it is used with -, it should change to the previous directory. 
    else if(strcmp(args[1], "-") == 0)
    {
        if(strlen(lwd) == 0)
        {
            printf("no previous directory\n");
            exit(1);
        }

        path = lwd;
    }
    else
    {
        path = args[1];
    }

    // change dir 
    if(chdir(path) != 0)
    {
        printf("cd failed\n");
        exit(1);
    }

    // lwd is previous dir before cd as we got currDir before we did chdir
    strcpy(lwd, currDir);
}

// 4. commands with pipe functions
bool command_with_pipes(char line[])
{
    return (strchr(line, '|') != NULL);
}

void trimWhitespace(char **str_ptr)
{
    char* str = *str_ptr;

    // leading whitespace
    while(*str == ' ') str++;

    char *end = str + strlen(str)-1;
    while(end > str && *end == ' ')
    {
        *end = '\0';
        end--;
    }

    *str_ptr = str;
}

void parse_pipes(char line[], char *commands[], int *commandCount)
{
    // same way as parsing command
    char *token = strtok(line, "|");
    *commandCount = 0;

    while(token != NULL && *commandCount < MAX_ARGS-1)
    {
        trimWhitespace(&token);
        commands[(*commandCount)] = token;
        (*commandCount)++;
        token = strtok(NULL, "|");
    }

    commands[*commandCount] = NULL;
}

void launch_pipes(char *commands[], int commandCount)
{
    // command1 stdout --> command2 stdin
    // command2 stdout --> command3 stdin
    // do it iteratively, maybe sequentially?

    /*
    command1 writes into the pipe for (1→2)
    command2 reads from previous pipe (1→2), and writes into a new pipe (2→3)
    command3 reads from previous pipe (2→3)

    fd[0] = read end;
    fd[1] = write end;
    */

    int prev_pipe_read = -1;

    for(int i = 0; i < commandCount; i++)
    {
        int fd[2] = {0, 0};
        // printf("commands[%d] : %s\n", i, commands[i]);

        if(i < (commandCount) - 1) // cmd 1 | cmd 2 | cmd 3 etc , we create n - 1 pipes
        {
            int piperc = pipe(fd);
            if(piperc < 0)
            {
                perror("pipe failed\n");
                exit(EXIT_FAILURE);
            }
        }

        int rc = fork();
        if(rc == 0)
        {
            // child process
            // if have previous pipe, connect the read end of that to stdin
            if(prev_pipe_read != -1)
            {
                dup2(prev_pipe_read, STDIN_FILENO);
                close(prev_pipe_read);
            }

            // if not last command, have stdout into write end of pipe
            if(i < (commandCount) - 1)
            {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }

            // parse command
            char *args[MAX_ARGS];
            int argsc;
            /*parse_command(commands[i], args, &argsc);
            for(int a = 0; a < argsc; a++)
            {
                printf("args[%d] = %s\n", a, args[a]);
            }
            printf("\n");*/

            if(command_with_redirection(commands[i])) // w redirection
            {
                // Parse for redirection and remove from args
                printf("\nEntering pipe with redirection\n");
                parse_command(commands[i], args, &argsc);
                /*for(int a = 0; a < argsc; a++)
                {
                    printf("args[%d] = %s\n", a, args[a]);
                }*/
                printf("\n");
                bool outputOverwrite = false;
                bool outputAppend = false;
                bool inputRedirect = false;


                for(int j = 0; j < argsc; j++)
                {
                    if(strcmp(args[j], ">>") == 0)
                    {
                        outputAppend = true;
                        break;
                    }
                    if(strcmp(args[j], ">") == 0)
                    {
                        outputOverwrite = true;
                        break;
                    }
                    if(strcmp(args[j], "<") == 0)
                    {
                        inputRedirect = true;
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
            else // normal
            {
                parse_command(commands[i], args, &argsc);
                /*for(int a = 0; a < argsc; a++)
                {
                    printf("args[%d] = %s\n", a, args[a]);
                }
                printf("\n");*/
                child(args, argsc);
            }

            // at this point exec should have replaced the code segment
            // if it reaches this point it failed
            perror("exec in launch failed\n");
            exit(EXIT_FAILURE);
        }
        else if(rc < 0)
        {
            perror("fork failed\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            // parent process
            
            if(prev_pipe_read != -1)
            {
                close(prev_pipe_read);
            }

            if(i < (commandCount)-1)
            {
                close(fd[1]);
                prev_pipe_read = fd[0];
            }
        }
    }

    for(int i = 0; i < commandCount; i++)
    {
        wait(NULL);
    }
}

bool command_with_batch(char line[])
{
    // check for ;
    return (strchr(line, ';') != NULL);
}

void parse_batch(char line[], char *batched_commands[], int *batchedCommandCount)
{
    char *token = strtok(line, ";");
    *batchedCommandCount = 0;

    while(token != NULL && *batchedCommandCount < MAX_ARGS-1)
    {
        trimWhitespace(&token);
        batched_commands[(*batchedCommandCount)] = token;
        (*batchedCommandCount)++;
        token = strtok(NULL, ";");
    }

    batched_commands[*batchedCommandCount] = NULL;
}

// will need lwd as would have to handle mkdir and cd...
void launch_batch(char *batched_commands[], int batchedCommandCount, char lwd[])
{
    // for every batched command we need to check
    // if it is cd or not first
    // if(pipe)
    // else if(redirection)
    // else --> normal

    // what i need is a char *commands[] if i get a pipe command
    

    for(int i = 0; i < batchedCommandCount; i++)
    {
        char* command_line = batched_commands[i];

        char* args[MAX_ARGS];
        int argsc = 0;

        char line_copy[MAX_LINE];
        strncpy(line_copy, command_line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        if(is_cd(line_copy))
        {
            parse_command(line_copy, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        else if(command_with_pipes(line_copy))
        {
            char* commands[MAX_ARGS];
            int commandCount;

            char pipe_copy[MAX_LINE];
            strncpy(pipe_copy, command_line, sizeof(pipe_copy) - 1);
            pipe_copy[sizeof(pipe_copy) - 1] = '\0';

            parse_pipes(pipe_copy, commands, &commandCount);
            launch_pipes(commands, commandCount);
        }
        else if(command_with_redirection(line_copy))
        {
            parse_command(line_copy, args, &argsc);
            launch_program_with_redirection(args, argsc);
        }
        else
        {
            parse_command(line_copy, args, &argsc);
            launch_program(args, argsc);
        }

        reap();
    }
}