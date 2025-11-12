#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);///Implement this function: initializes lwd with the cwd (using getcwd)


    // we call this in resolve now

    //Stores pointers to command arguments.
    ///The first element of the array is the command name.
    //char *args[MAX_ARGS];

    ///Stores the number of arguments
    //int argsc;

    while (1)
    {

        read_command_line(line, lwd); ///Notice the additional parameter (required for prompt construction)
        
        resolve(line, lwd);
    }
    

    return 0;
}


// going straight to nested subshells
// replace the whole main to just call a recursive func that checks

/*
// batched command can have cd involved...
if(command_with_subshell(line))
{
    // our helper functions will have to do it sequentially until the end of the string
    resolve_command_with_subshell(line);

}
else if(command_with_batch(line))
{
    char *batched_commands[MAX_ARGS];
    int batchedCommandCount;
    parse_batch(line, batched_commands, &batchedCommandCount);
    launch_batch(batched_commands, batchedCommandCount, lwd);
    reap();
}
else if(is_cd(line)){///Implement this function
    parse_command(line, args, &argsc);
    run_cd(args, argsc, lwd); ///Implement this function
}
else if(command_with_pipes(line))
{
    char *commands[MAX_ARGS];
    int commandCount;
    parse_pipes(line, commands, &commandCount);
    launch_pipes(commands, commandCount);
    reap();
}
else if(command_with_redirection(line)){
    ///Command with redirection
    parse_command(line, args, &argsc);
    launch_program_with_redirection(args, argsc);
    reap();
}
else ///Basic command
{
    parse_command(line, args, &argsc);
    launch_program(args, argsc);
    reap();
}
*/