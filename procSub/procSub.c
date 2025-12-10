#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// Function to execute a command
void child(char *args[], int argsc) {
    execvp(args[0], args);
    perror("execvp failed");
    exit(1);
}

// Helper function to remove redirection tokens
int remove_redirection_tokens(char *args[], int argsc, int index) {
    // Remove both the redirection token and the filename
    for(int i = index; i < argsc - 2; i++) {
        args[i] = args[i + 2];
    }
    args[argsc - 2] = NULL;
    return argsc - 2;
}

// Check if a string is a process substitution pattern
int is_process_substitution(const char *str) {
    return (strlen(str) > 3 && str[0] == '<' && str[1] == '(' && str[strlen(str)-1] == ')');
}

// Extract command from process substitution pattern
char* extract_subcommand(const char *pattern) {
    if (!is_process_substitution(pattern)) {
        return NULL;
    }
    
    // Extract command between <( and )
    int len = strlen(pattern);
    char *command = malloc(len - 2);  // Subtract 3 for <( and ), plus 1 for null terminator
    
    if (!command) {
        return NULL;
    }
    
    strncpy(command, pattern + 2, len - 3);
    command[len - 3] = '\0';
    
    return command;
}

// Execute process substitution and return a file descriptor
int execute_process_substitution(const char *subcommand) {
    int pipefd[2];
    
    // Create pipe for communication
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute the subcommand
        close(pipefd[0]);  // Close read end
        
        // Redirect stdout to the pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        
        // Parse and execute the subcommand
        // This is simplified - you'd need your own command parser here
        char *args[] = {"sh", "-c", (char*)subcommand, NULL};
        execvp("sh", args);
        
        perror("execvp for subcommand");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        close(pipefd[1]);  // Close write end
        return pipefd[0];  // Return read end as file descriptor
    } else {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
}

// Updated version of your function that handles both files and process substitution
void child_with_input_redirected(char *args[], int argsc, char lwd[]) {
    char* inputSource = NULL;
    int fd;
    
    // Find input redirection
    for(int i = 0; i < argsc; i++) {
        if(strcmp(args[i], "<") == 0 && i+1 < argsc) {
            inputSource = args[i+1];
            argsc = remove_redirection_tokens(args, argsc, i);
            break;
        }
    }
    
    if(inputSource == NULL) {
        printf("No input source specified after <\n");
        exit(1);
    }
    
    // Check if it's a process substitution or a regular file
    if (is_process_substitution(inputSource)) {
        // Handle process substitution: <(command)
        printf("Processing command substitution: %s\n", inputSource);
        
        // Extract the command from <(command)
        char *subcommand = extract_subcommand(inputSource);
        if (!subcommand) {
            printf("Invalid process substitution format\n");
            exit(1);
        }
        
        printf("Subcommand to execute: %s\n", subcommand);
        
        // Execute the subcommand and get a file descriptor
        fd = execute_process_substitution(subcommand);
        free(subcommand);
        
        if (fd == -1) {
            printf("Failed to execute process substitution\n");
            exit(1);
        }
    } else {
        // Handle regular file
        printf("Reading from file: %s\n", inputSource);
        fd = open(inputSource, O_RDONLY);
        
        if(fd == -1) {
            printf("Error opening input file: %s\n", inputSource);
            exit(1);
        }
    }
    
    // Redirect stdin from the file descriptor
    int dup2rc = dup2(fd, STDIN_FILENO);
    if(dup2rc == -1) {
        printf("Error with dup2 for input redirection\n");
        close(fd);
        exit(1);
    }
    
    close(fd);
    child(args, argsc);
}

// Test the implementation
int main() {
    // Test case 1: Regular file input redirection
    printf("=== Test 1: Regular file input ===\n");
    char *test1[] = {"wc", "<", "../txt/phrases.txt", NULL};
    
    pid_t pid1 = fork();
    if (pid1 == 0) {
        child_with_input_redirected(test1, 3, ".");
        exit(0);
    }
    waitpid(pid1, NULL, 0);
    
    printf("\n=== Test 2: Process substitution ===\n");
    // Test case 2: Process substitution
    char *test2[] = {"wc", "<", "<(cat ../txt/phrases.txt)", NULL};
    
    // First create the test file
    system("mkdir -p txt && echo -e 'Hello\\nWorld\\nTest' > txt/phrases.txt");
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        child_with_input_redirected(test2, 3, ".");
        exit(0);
    }
    waitpid(pid2, NULL, 0);
    
    printf("\n=== Test 3: Process substitution with pipeline ===\n");
    // Test case 3: More complex process substitution
    char *test3[] = {"wc", "<", "<(cat ../txt/phrases.txt | grep 'o')", NULL};
    
    pid_t pid3 = fork();
    if (pid3 == 0) {
        child_with_input_redirected(test3, 3, ".");
        exit(0);
    }
    waitpid(pid3, NULL, 0);
    
    return 0;
}