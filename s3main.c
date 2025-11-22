#include "s3.h"

int main(int argc, char *argv[]){

    ///Stores the command line input
    char line[MAX_LINE];

    ///The last (previous) working directory 
    char lwd[MAX_PROMPT_LEN-6]; 

    init_lwd(lwd);

    while(1)
    {
        read_command_line(line, lwd);

        if(is_batch_command(line))
        {
            char *batches[MAX_ARGS];
            int batchc;
            tokenize_batch_command(line, batches, &batchc);

            for(int i=0; i<batchc; i++)
            {
                process_single_command(batches[i], lwd);
            }
        }
        
        else
        {
            process_single_command(line, lwd);
        } 
        reap();
    }
    return 0;
}
