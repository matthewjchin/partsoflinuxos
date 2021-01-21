# Project 2: Command Line Shell

See: https://www.cs.usfca.edu/~mmalensek/cs326/assignments/project-2.html

--------------------------------------------------------------------------------------
**************************************************************************************
--------------------------------------------------------------------------------------

For shell.c

print_prompt:
Prints out the prompt as the user first opens up shell. Returns:
    -Command Number
    -Hostname of machine
    -Username of who is using the virtual machine
    -Current directory that the shell is currently on at moment

execute_pipe:
-A core type of the Unix system. This allows for redirection of pipes
in shell so that existing utilities in shell are combined instead of
added over and over again by user input. 
-Checks to see if the command after the pipe character is found that 
is valid and can be run in the line of tokens. 

expansion:
Returns a string array based on what input is put in via string and 
beginning with a dollar sign character '$'
    -$PATH: finds the executable directories open during shell use
    -$SHELL: shell environment run at time of login

exit_shell:
Exits out of shell whenever user types "exit" command
Closes out of shell and gives message to user indicating exiting of shell

sigint_handler:
Does not terminate the shell when user inputs ^C via keyboard
    -Responsible for all signal handling and keeps running when command is typed
    -First called in the main function whenever above condition above is true

deal_with_quotes:
Checks the single and double quotation marks in shell input
Check with curr_tok
Check with tokens

main:
Calls when to print out prompts from shell
    -Gets commands from line based on user input in the shell
    -Calls functions when needed to based on Linux command input
    -Keeps number of commands to shell and updates when needed


--------------------------------------------------------------------------------------
**************************************************************************************
--------------------------------------------------------------------------------------

For history.c

get_history:
Takes all of the elements in the array of structs that will return 
the last 100 commands entered into the shell. Prints out the history entries.
    -Keep track of bang (!) sign
    -Keep track of commands instead of returning the last 100 when number inputted
    for the last commands into shell until the number the user input to shell
    -For loop iterates through the total number of elements in the line of
    the array
    -Prints out the appropriate number of the operation at the time 
    -Tells user that history is not implemented yet if there is one aspect 
    of the history command that has not been programmed

get_double_bang:
Function called if "!!" is found. Should execute that command in addition 
to returning the command that the user put into shell, if that command is valid.

get_range_bang:
Checks to see if the number that the user put in is valid and if so returns the last 
x number of commands as put in by the user, including the last command.

add_to_history:
Adds command count index and command to the struct
Arranges space if the command count is 100 or more to return the last 
100 commands entered into the shell, excluding the others before the last 100. 

--------------------------------------------------------------------------------------
**************************************************************************************
--------------------------------------------------------------------------------------

For tokenizer.c

next_token:
Retrieves the next token from a string.
Parameters:
    - str_ptr: maintains context in the string, i.e., where the next token in the
    string will be. If the function returns token N, then str_ptr will be
    updated to point to token N+1. To initialize, declare a char * that points
    to the string being tokenized. The pointer will be updated after each
    successive call to next_token.
    - delim: the set of characters to use as delimiters
Returns: char pointer to the next token in the string.
