# S3 Shell - Custom Bash Implementation

## Features

### Core Functionalities
1. **Basic Command Execution** - Execute standard system commands
2. **Input/Output Redirection** - Support for `>`, `<`, `>>` operators
3. **Directory Navigation** - `cd` command with tilde (`~`) expansion for home directory
4. **Piping** - Multiple pipe support (`|`) for command chaining
5. **Batched Commands** - Execute multiple commands sequentially (`;`)
6. **Subshells** - Execute commands in subshells with `$(...)` syntax
7. **Nested Subshells** - Support for multiple levels of subshell nesting
8. **Globbing/Pattern Matching** - Support for wildcards:
   - `*` - Match any sequence of characters
   - `?` - Match any single character
   - `[]` - Match character ranges/sets

### User Interface Features
- **Tab Autocompletion** - Intelligent command/path completion
  - Single match: Auto-complete immediately
  - Multiple matches: Display all available options
  - Current limitation: Autocomplete works within current directory only
- **Terminal Control** - Custom input handling using `termios.h`

## Installation & Compilation

### Prerequisites
- GCC compiler
- Linux/Unix-based system
- Standard C libraries

### Build Instructions
```bash
# Clone the repository
git clone <repository-url>
cd Software-Systems-Assignment-1

# Compile the shell
gcc *.c -o s3 

# Run the shell
./s3