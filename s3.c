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
        printf("exiting shell\n");
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
        printf("entering child process\n");
        child(args, argsc);
    }
    else
    {
        wait(NULL);
        printf("parent process now\n");
    }
    return;
}

bool command_with_redirection(char line[])
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
void launch_program_with_redirection(char *args[], int argsc)
{

    if(strcmp(args[0], "exit") == 0) // args[0] == "exit"
    {
        printf("exiting shell\n");
        exit(0);
    }

    int rc = fork();
    if(rc < 0)
    {
        //fork failed
        printf("fork failed\n");
        exit(1);
    }
    else if(rc == 0)
    {
        //child process
        printf("entering redirection child process\n");
        
        char *redirection_type = NULL;
        char *file_name = NULL;
        int redirection_index = -1;

        //for loop to look for the redirection symbols and file name
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
                    perror("missing filename after redirection symbol");
                    exit(1);
                }
            }
        }
        //call appropriate redirection function to redirect I/O based on the symbol found
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
        
        //terminate args before the redirection symbol
        args[redirection_index] = NULL; 
        
        char *programmeName = args[0];
        
        //execute the command with redirected I/O
        if(execvp(programmeName, args) == -1)
        {
            printf("child function failed\n");
            exit(1);
        }
    }
    else
    {
        wait(NULL);
        printf("parent process now (redirection)\n");
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
        perror("getcwd failed\n");
        return;
    }
    // case 1 : cd and cd ~ (return to home directory)
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
        perror("s3: cd failed");
    }
}