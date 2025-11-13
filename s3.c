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
        if(strlen(token) > 0)
        {
            batched_commands[(*batchedCommandCount)] = token;
            (*batchedCommandCount)++;
        }
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
        //printf("Command[%d]: %s (addr %p)\n", i, batched_commands[i], batched_commands[i]);
        char* command_line = batched_commands[i];

        // ** make a copy since parse functions use strtok
        char line_copy[MAX_LINE];
        strncpy(line_copy, command_line, sizeof(line_copy)-1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        char* args[MAX_ARGS];
        int argsc = 0;

        if(is_cd(line_copy))
        {
            // maek another copy for parse command;
            char cd_copy[MAX_LINE];
            strncpy(cd_copy, line_copy, sizeof(cd_copy) - 1);
            cd_copy[sizeof(cd_copy) - 1] = '\0';

            parse_command(line_copy, args, &argsc);
            run_cd(args, argsc, lwd);
        }
        else if(command_with_pipes(line_copy))
        {
            char* commands[MAX_ARGS];
            int commandCount;

            // copy for pipes
            char pipe_copy[MAX_LINE];
            strncpy(pipe_copy, command_line, sizeof(pipe_copy) - 1);
            pipe_copy[sizeof(pipe_copy) - 1] = '\0';

            parse_pipes(pipe_copy, commands, &commandCount);
            launch_pipes(commands, commandCount);
        }
        else if(command_with_redirection(line_copy))
        {
            // copy for redirect parse
            char redir_copy[MAX_LINE];
            strncpy(redir_copy, line_copy, sizeof(redir_copy) - 1);
            redir_copy[sizeof(redir_copy) - 1] = '\0';

            parse_command(line_copy, args, &argsc);
            launch_program_with_redirection(args, argsc);
        }
        else
        {
            // make copy for parse command too
            char normal_copy[MAX_LINE];
            strncpy(normal_copy, line_copy, sizeof(normal_copy) - 1);
            normal_copy[sizeof(normal_copy) - 1] = '\0';

            parse_command(line_copy, args, &argsc);
            launch_program(args, argsc);
        }

        reap();
    }
}


bool command_with_subshell(char line[])
{
    // check for ( and ).
    return strchr(line, '(') != NULL && strchr(line, ')') != NULL;
}

// returns the next index to continue going down sequentially
int parse_what_is_in_subshell(int startIdx, char line[], char *subshell_content[])
{
    int balance = 0;
    int endIdx = -1;
    for (int i = startIdx; line[i] != '\0'; i++) {
        if (line[i] == '(') balance++;
        else if (line[i] == ')') balance--;

        if(balance == 0)
        {
            endIdx = i;
            break;
        }
    }
    
    int len = endIdx - startIdx - 1;
    *subshell_content = malloc(len + 1);
    strncpy(*subshell_content, line + startIdx + 1, len);
    (*subshell_content)[len] = '\0';

    return endIdx + 1;
}

void launch_subshell(char *subshell_command, char* lwd)
{
    int rc = fork();

    if(rc == 0)
    {
        resolve(subshell_command, lwd);
        exit(0);
    }
    else if(rc < 0)
    {
        perror("failure forking in subshell");
        exit(1);
    }
    else
    {
        wait(NULL);
    }
}

void launch_subshell_with_redirection(char *subshell_command, char *redirect_cmd, char *lwd)
{
    int rc = fork();

    if(rc == 0)
    {
        // child process --> set up the redirection before running

        // parse redirection command
        char *args[MAX_ARGS];
        int argsc;

        char redirect_copy[MAX_LINE];
        strncpy(redirect_copy, redirect_cmd, sizeof(redirect_copy) - 1);
        redirect_copy[sizeof(redirect_copy) - 1] = '\0';

        parse_command(redirect_copy, args, &argsc);

        // check on type of redirection

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
            else if(strcmp(args[i], ">") == 0)
            {
                outputOverwrite = true;
                break;
            }
            else if(strcmp(args[i], "<") == 0)
            {
                inputRedirect = true;
                break;
            }
        }

        // fd redirection
        char *filename = NULL;
        for(int i = 0; i < argsc; i++)
        {
            if((strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 || strcmp(args[i], "<") == 0) &&
            i+1 < argsc)
            {
                filename = args[i+1];
                break;
            }
        }

        int fd;
        if(inputRedirect)
        {
            fd = open(filename, O_RDONLY);
            if(fd == -1)
            {
                perror("error opening input file \n");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
        }
        else if(outputAppend)
        {
            fd = open(filename, O_WRONLY | O_CREAT | O_APPEND , 0644);
            if(fd == -1)
            {
                perror("error opening output file \n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
        }
        else if(outputOverwrite)
        {
            fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC , 0644);
            if(fd == -1)
            {
                perror("error opening output file \n");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
        }
        close(fd);

        resolve(subshell_command, lwd);
        exit(0);
    }
    else if(rc < 0)
    {
        perror("failure forking in subshell w redirection\n");
        exit(0);
    }
    else
    {
        wait(NULL);
    }
}
// resolve subshell recursively sequentially i guess
void resolve_command_with_subshell(char line[], char *lwd)
{
    int i = 0;
    int totalLength = strlen(line);

    while(i < totalLength)
    {
        if(line[i] == '(')
        {
            char *subshell_content = NULL;
            int next = parse_what_is_in_subshell(i, line, &subshell_content);

            // check if redirection after subshell
            int redirect_start = next;
            while(redirect_start < totalLength && line[redirect_start] == ' ')
            {
                redirect_start++;
            }

            // check for redirect operators
            if(redirect_start < totalLength && 
            (line[redirect_start] == '>' || line[redirect_start] == '<'))
            {
                // extract redirection
                char redirect_cmd[MAX_LINE];
                int redirect_len = 0;

                while(redirect_start < totalLength && line[redirect_start] != ';' &&
                line[redirect_start] != '(')
                {
                    redirect_cmd[redirect_len++] = line[redirect_start++];
                }
                redirect_cmd[redirect_len] = '\0';

                launch_subshell_with_redirection(subshell_content, redirect_cmd, lwd);
                i = redirect_start;
            }
            else
            {
                launch_subshell(subshell_content, lwd);
                i = next;
            }
            free(subshell_content);
        }
        else
        {
            int start = i;
            while(i < totalLength && line[i] != '(') i++;

            int len = i - start;
            if(len > 0)
            {
                char* segment = malloc(len + 1);
                strncpy(segment, line + start, len);
                segment[len] = '\0';
                
                // trim whitespace
                char *trimmed = segment;
                trimWhitespace(&trimmed);

                if(strlen(trimmed) > 0)
                {
                    resolve(trimmed, lwd);
                }

                free(segment);
            }
        }
    }
}

void resolve(char line[], char *lwd)
{
    // make a working copy
    char line_copy[MAX_LINE];
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    char *args[MAX_ARGS];
    int argsc;

    if(command_with_subshell(line_copy))
    {
        // our helper functions will have to do it sequentially until the end of the string
        resolve_command_with_subshell(line_copy, lwd);
    }
    else if(command_with_batch(line_copy))
    {
        char *batched_commands[MAX_ARGS];
        int batchedCommandCount;
        parse_batch(line_copy, batched_commands, &batchedCommandCount);
        launch_batch(batched_commands, batchedCommandCount, lwd);
        reap();
    }
    else if(is_cd(line_copy)){///Implement this function
        char cd_copy[MAX_LINE];
        strncpy(cd_copy, line_copy, sizeof(cd_copy) - 1);
        cd_copy[sizeof(cd_copy) - 1] = '\0';

        parse_command(cd_copy, args, &argsc);
        run_cd(args, argsc, lwd); ///Implement this function
    }
    else if(command_with_pipes(line))
    {
        char *commands[MAX_ARGS];
        int commandCount;

        char pipe_copy[MAX_LINE];
        strncpy(pipe_copy, line_copy, sizeof(pipe_copy) - 1);
        pipe_copy[sizeof(pipe_copy) - 1] = '\0';

        parse_pipes(pipe_copy, commands, &commandCount);
        launch_pipes(commands, commandCount);
        reap();
    }
    else if(command_with_redirection(line)){
        char redir_copy[MAX_LINE];
        strncpy(redir_copy, line_copy, sizeof(redir_copy) - 1);
        redir_copy[sizeof(redir_copy) - 1] = '\0';

        ///Command with redirection
        parse_command(redir_copy, args, &argsc);
        launch_program_with_redirection(args, argsc);
        reap();
    }
    else ///Basic command
    {
        char normal_copy[MAX_LINE];
        strncpy(normal_copy, line_copy, sizeof(normal_copy) - 1);
        normal_copy[sizeof(normal_copy) - 1] = '\0';

        parse_command(normal_copy, args, &argsc);
        launch_program(args, argsc);
        reap();
    }
}