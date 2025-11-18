#include "s3.h"

///Updated shell prompt has to include the working directory
void construct_shell_prompt(char shell_prompt[], const char lwd[])
{
    char cwd[MAX_PROMPT_LEN];   //buffer to store current working directory
    const char *to_display = NULL;  

    if(getcwd(cwd, MAX_PROMPT_LEN)!= NULL)  //getcwd function finds the current working directory and returns it in cwd
    {
        to_display = cwd;  //display cwd 
    }
    else
    {
        to_display = "error displaying cwd";
    }

    snprintf(
        shell_prompt,
        MAX_PROMPT_LEN,
        "[S3:%s]$",  //restricts the print format
        to_display 
    );
}

///Prints a shell prompt and reads input from the user
void read_command_line(char line[], const char lwd[])
{
    char shell_prompt[MAX_PROMPT_LEN];  //buffer to store the shell prompt
    construct_shell_prompt(shell_prompt, lwd);  //past lwd to the prompt constructor
    printf("%s", shell_prompt);

    ///See man page of fgets(...)
    if (fgets(line, MAX_LINE, stdin) == NULL)   //reads chars from stdin, stores then in line array
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
    char *token = strtok(line, " ");    // "hello world\0" --> "hello\0" "world\0"
    *argsc = 0;
    while (token != NULL && *argsc < MAX_ARGS - 1)
    {
        args[(*argsc)++] = token;
        token = strtok(NULL, " ");
    }
    
    args[*argsc] = NULL;    //args must be null terminated
}

void child(char *args[], int argsc, int p_in[2], int p_out[2])  //augmented to work with pipes
{
    if (p_in[0] > 0)    //program reads from pipe
    {
        dup2(p_in[0], STDIN_FILENO);    //direct stdin to input pipe read end
        close(p_in[0]);
        close(p_in[1]);
    }

    if (p_out[1] > 0)   //program writes to pipe
    {
        dup2(p_out[1], STDOUT_FILENO);  //direct stdout to pipe write end
        close(p_out[0]);
        close(p_out[1]);
    }

    char *programmeName = args[0];

    if(execvp(programmeName, args) == -1)
    {
        printf("child function failed\n");
        exit(1);
    }
}

void launch_program(char *args[], int argsc, int p_in[2], int p_out[2]) //augmented to work with pipes
{
    if(strcmp(args[0], "exit") == 0)    // args[0] == "exit"
    {
        printf("exiting shell\n");
        exit(0);
    }
    int rc = fork();
    
    if (rc < 0)
    {
        perror("fork failed\n");    
    }
    else if (rc == 0)   //child process
    {
        child(args, argsc, p_in, p_out);
    }
    wait(NULL);
    printf("parent process now\n");
}

bool is_redirection(char line[])
{   
    //checks for characters '<' and '>'
    if(strchr(line, '>') != NULL || strchr(line, '<') != NULL)
    {
        return true;
    }

    return false;
}

void child_with_output_overwrite(char *file_name)
{
    //  handles '>' (overwrite)
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
    // handles '<' (take input from file)
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
//file redirection takes precedence over piping
void launch_program_with_redirection(char *args[], int argsc, int p_in[2], int p_out[2])
{
    if(strcmp(args[0], "exit") == 0) // args[0] == "exit"
    {
        printf("exiting shell\n");
        exit(0);
    }
    
    int rc = fork();
    if (rc < 0)
    {
        perror("fork failed\n");
        return;
    }
    else if (rc == 0)   //child
    {
        //can be overwritten later on by redirection
        if (p_in[0] > 0)    //program reads from pipe
        {
            dup2(p_in[0], STDIN_FILENO);    //direct stdin to input pipe read end
            close(p_in[0]);
            close(p_in[1]);
        }

        if (p_out[1] > 0)   //program writes to pipe
        {
            dup2(p_out[1], STDOUT_FILENO);  //direct stdout to pipe write end
            close(p_out[0]);
            close(p_out[1]);
        }

        char *redirection_type = NULL;
        char *file_name = NULL;
        int redirection_index = -1;
    
        //for loop to look for the redirection symbols and file name
        for(int i = 0; i < argsc; i++)
        {
            if (strcmp(args[i], ">>") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0) 
            {
                redirection_type = args[i];
                if (i + 1 < argsc)
                {
                    file_name = args[i+1];
                    redirection_index = i;
                    break;
                }
                else
                {
                    perror("missing filename after redirection symbol");
                    exit(1);
                }
            }
        }
        //call appropriate redirection function to redirect I/O based on the symbol found (will overwrite the pipes)
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
            args[redirection_index] = NULL;
        }
        execvp(args[0], args);
        perror("execvp failed");
        exit(1);
    }
    else    //back to parent function
    {
        if (p_in[0] > 0)    //close input read end
        {
            close(p_in[0]);
        }
        if (p_out[1] > 0)   //close output write end
        {
            close(p_out[1]);
        }
        wait(NULL);
        printf("parent process(redirection)\n");
    }
}

//function required for cd ~ which retraces previous directory
void init_lwd(char lwd[])
{
    if (getcwd(lwd, MAX_PROMPT_LEN) == NULL)    //writes pathname of cwd to lwd, returns NULL if unsuccessful
    {
        //if getcwd fails, fall back to home directory
        char *home_dir = getenv("HOME");
        //if able to retrieve the home directory
        if (home_dir != NULL) {
            strncpy(lwd, home_dir, MAX_PROMPT_LEN - 1); //copies home_dir to lwd
            lwd[MAX_PROMPT_LEN - 1] = '\0'; //forces the last character to be NULL
        } 
        //if not able to retrieve the home directory, fall back to root directory /
        else 
        {
            strcpy(lwd, "/");
        }
        perror("init_lwd: getcwd failed");
    }
}

bool is_cd(const char line[])
{
    if(line[0] == 'c' && line[1] == 'd')  // " " refers to an array of chars while ' ' refers to a char
        if(line[2] == ' ' || line[2] == '\0'){  //ensure theres a space or null after command cd
            return true;
        }
    return false;    
}

void run_cd(char *args[], int argsc, char lwd[])
{
    char *target_directory = NULL;  
    char old_cwd[MAX_PROMPT_LEN];

    //getcwd stores current working directory into old_cwd, and returns NULL if failed
    if(getcwd(old_cwd, MAX_PROMPT_LEN) == NULL) 
    {
        perror("getcwd failed");
        return;
    }
    //case 1 : cd and cd ~ (return to home directory)
    else if(argsc == 1 || strcmp(args[1], "~") == 0)
    {
        target_directory = getenv("HOME");  //getenv searches the environment and returns a pointer pointing to the string containing the path
        if(target_directory == NULL)
        {
            fprintf(stderr, "HOME environment not set \n");
            return;
        }
    }
    //case 2 : cd - (return to previous directory)
    else if(strcmp(args[1], "-") == 0)
    {
        target_directory = lwd;
        if (lwd[0] == '\0' || strcmp(lwd, old_cwd) == 0)    //if no last working directory
        {
            fprintf(stderr, "no last working directory recorded \n");
            return;
        }
    }
    //case 3 : navigate to given directory
    else
    {
        target_directory = args[1];
    }

    if (target_directory != NULL && chdir(target_directory)==0) //chdir changes directory to target_directory
    {
        strncpy(lwd, old_cwd, MAX_PROMPT_LEN);  //update the last working directory
        lwd[MAX_PROMPT_LEN - 1] = '\0'; 
        
        //prints feedback if cd - was used
        if (argsc > 1 && strcmp(args[1], "-") == 0) {
            char new_cwd[MAX_PROMPT_LEN];
            if (getcwd(new_cwd, MAX_PROMPT_LEN) != NULL) {
                printf("%s\n", new_cwd);
            }
        }
    } 
    else if (target_directory != NULL) 
    {
        perror("cd failed");
    }
}

bool is_pipe(const char line[])
{
    if (strchr(line, '|') != NULL)  //checks for |, return NULL if not present
    {
        return true;
    }
    return false;
}

//similar to parse function but looks for '|' to split the commands instead
void tokenize_pipeline(char line[], char *commands[], int *commandc)
{
    char *token = strtok(line, "|");    //splits tokens by "|"
    *commandc = 0;
    while (token != NULL && *commandc < MAX_ARGS - 1)
    {
        commands[(*commandc)++] = token;   //stores the fragmented commands in args
        token = strtok(NULL, "|" ); //runs until NULL is encountered (end of command)
    }
    commands[*commandc] = NULL; 
}

void launch_program_with_pipe(char *commands[], int commandc)
{
    printf("entering pipe process\n");
    //p_in carries the output from previous command to the current command, hence it must be outside the loop
    int p_in[2] = {-1, -1};  //initialise array to hold file descriptors, one read one write
    for(int i=0; i < commandc; i++)    //iterate through each token
    {
        char *current_command = commands[i];
        char command_copy[MAX_LINE];    
        strncpy(command_copy, current_command, MAX_LINE - 1);   //create a copy of the current command
        command_copy[MAX_LINE - 1] = '\0';

        int argsc;
        char *args[MAX_ARGS];;
        parse_command(command_copy, args, &argsc);  //need to parse the current command to split into indiv args(since tokenizing only splits based on "|", not " ")
        
        if(argsc == 0)
        {
            perror("empty command in pipeline");
            return;
        }

        //p_out carries the output from this command to the next command(if any)
        int p_out[2] = {-1, -1};
        if (i < argsc -1)   //if not the last command
        {
            if (pipe(p_out) == -1) //creates output pipe and stores file descriptors in p_out, returns -1 if failed
            {
                perror("output pipe creation failed"); 
                if(p_in[0] != -1)
                {
                    close(p_in[0]); //close read end of input pipe
                }
                return;
            }
        }
        if (is_redirection(command_copy))
        {
            launch_program_with_redirection(args, argsc, p_in, p_out);
        }
        else 
        {
            launch_program(args, argsc, p_in, p_out);
        }
        
        if (p_in[0] != -1)
        {
            close(p_in[0]);
        }
        if (i < argsc - 1)
        {
            close(p_out[1]);
        }
        
        p_in[0] = p_out[0];  //map old pipe to the new pipe
        p_in[1] = p_out[1];
    }
    if (p_in[0] != -1)
    {
        close(p_in[0]);
    }
    printf("exiting pipefunction\n");
}
