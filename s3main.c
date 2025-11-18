#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    char *args[MAX_ARGS];
    char *commands[MAX_ARGS];

    ///Stores the number of arguments
    int argsc;
    int commandc;   //for pipes

    while (1) {

        read_command_line(line, lwd);

        if(is_cd(line))
        {
            parse_command(line, args, &argsc);
            run_cd(args, argsc, lwd); 
        }
        else if(is_pipe(line))  //check for presence of pipes first
        {
            tokenize_pipeline(line, commands, &commandc);
            launch_program_with_pipe(commands, commandc);
            reap();
        }
        else if(is_redirection(line))  
        {
           parse_command(line, args, &argsc);
           launch_program_with_redirection(args, argsc, NULL, NULL);
           reap();
        }
        else 
        {
           parse_command(line, args, &argsc);
           launch_program(args, argsc, NULL, NULL);
           reap();
        }
    }
    return 0;
}
