#include <glob.h>
#include <stdio.h>

int main()
{
    glob_t glob_result;

    // find all .c files
    int ret = glob("[a-g]*.c", 0, NULL, &glob_result);

    if(ret == 0)
    {
        printf("Found %zu files : \n", glob_result.gl_pathc);

        for(size_t i = 0; i < glob_result.gl_pathc; i++)
        {
            printf(" %s\n", glob_result.gl_pathv[i]);
        }

        globfree(&glob_result);
    }
    else if (ret == GLOB_NOMATCH)
    {
        printf("No matches found\n");
    }

    return 0;
}