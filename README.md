# Software-Systems-Assignment-1
Shell Implementation

** Take a look at testing.md for the tests carried out to test the functionality from 6-8.

S3 Shell able to handle the following
1. Basic Commands

2. Commands with redirection

3. Support for cd (Including tilde (~))

4. Commands with pipes

5. Batched commands

6. Subshells (except process substitution take a look in testing.md for the tests carried out)

7. Nested Subshells

8. Globbing ([], *, ? wildcards are handled)

9. User Interface - using the termios.h header file to allow for autocomplete with tab
- when there is 1 match, the command gets autocompleted
- when there is more than 1 match, all matches get displayed to the user
- currently only able to handle within the same direction for example:
    cd ../t (tab)
    where i have a dir called txt outside, will not autocomplete.
