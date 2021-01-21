#include <fcntl.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>

// Checks to see if there is debugging needed at any point in program - LOG and LOGP returning
//#include "debug.h"  
// Retrieves the history of all commands given to shell and corresponding pid number
#include "history.h"  
// Gets the time taken that shell is on and/or used to take time
#include "timer.h" 
// Tokenizes each string into memory word by word
#include "tokenizer.h"  

long command_ct;
/**
 * struct: command_line
 * ------------------------------------------------------
 * Takes the line of tokens from input
 * Checks to see if pipe exists
 * Returns anything to a random output file if the token 
 * is the last token and if it is found to be NULL. 
 * -------------------------------------------------------
 */
struct command_line {
    char **tokens;
    bool stdout_pipe;
    char *stdout_file;
};
struct command_line *cl;

// This is the struct from history.h file
int arr_ct = 0;

/**
 * print_prompt
 * -------------------------------------------------------
 * Print out the prompt which includes:
 * 
 * -Command Number via global variable count from history
 * -Hostname of virtual machine
 * -Username of who is using the virtual machine
 * -Current directory that the shell is in at moment
 * -------------------------------------------------------
 */
void print_prompt(){
    uid_t user_id = getuid();
    struct passwd *pw = getpwuid(user_id);
    char *buffer = malloc(sizeof(char) * 100);
    char *name = pw->pw_name;
    char *curr_dir = getcwd(buffer, 100);
    char host[HOST_NAME_MAX];

    /** 
     * Return the value of the gethostname
     * If 0 return the current directory located in
     * If -1 return message that hostname not found (but rare)
     */
    int x = gethostname(host, HOST_NAME_MAX);
    if(x == 0){
        printf("---[%ld|%s@%s:%s]---$ ", count, name, host, curr_dir);
    }
    fflush(stdout);
    free(buffer);
}

/**
 * execute_pipe
 * ---------------------------------------------------------------------
 * A core type of the Unix system. This allows for redirection of pipes
 * in shell so that existing utilities in shell are combined instead of
 * added over and over again by user input.
 * 
 * Checks to see if the command after the pipe character is found that 
 * is valid and can be run in the line of tokens. 
 * ---------------------------------------------------------------------
 */
void execute_pipe(struct command_line *commands){
    if(commands->stdout_pipe == false){
        if(commands->stdout_file != NULL){
            int open_flags = O_RDWR | O_CREAT | O_TRUNC;
            int open_perms = 0644;
            int fd = open(commands->stdout_file, open_flags, open_perms);
            if (fd == -1) {
                perror("open");
                return;
            }
            if (dup2(fd, fileno(stdout)) == -1) {
                perror("dup2");
                return;
            }
        }
        if(execvp(commands->tokens[0], commands->tokens) == -1){
            perror("execvp");
        }
        return;
    }

    int second[2];
    if(pipe(second) == -1){
        perror("pipe");
        return;
    }    
    pid_t pid = fork();
    if(pid < 0){
        perror("Forking failed.\n");
        return;
    }
    if(pid == 0){ 
        // if pid is the child
        dup2(second[1], fileno(stdout));
        close(second[0]);
        if(execvp(commands->tokens[0], commands->tokens) == -1){
            perror("execvp");
        }
    }
    else{ 
        // if pid is the parent
        dup2(second[0], STDIN_FILENO);
        close(second[1]);
        execute_pipe(++commands);
    }
}

/**
 * expansion
 * -----------------------------------------------------------
 * Returns a string array of chars with regards to retrieving
 * the name of the executable directories being opened
 *
 * $PATH - executable directories opened during shell use
 * $SHELL - environment variable of the shell at login time
 * 
 * Return the path of one of the two above
 * -----------------------------------------------------------
 */
char *expansion(char *str){
    size_t var_start = 0;
    var_start = strcspn(str, "$");
    if (var_start == strlen(str)) {
        /* No variable to replace */
        return NULL;
    }

    size_t var_len = strcspn(str + var_start, " \t\r\n\'\"");
    char *var_name = malloc(sizeof(char) * var_len + 1);
    if (var_name == NULL) {
        return NULL;
    }
    strncpy(var_name, str + var_start, var_len);
    var_name[var_len] = '\0';
    if (strlen(var_name) <= 1) {
        free(var_name);
        return NULL;
    }

    /* Use index 1 to ignore the '$' prefix */
    char *value = getenv(&var_name[1]);
    if (value == NULL) {
        // fprintf(stderr, "value was null\n");
        value = "";
    }
    free(var_name);

    /* Grab the size of the remaining string (after the $var): */
    size_t remain_sz = strlen(str + var_start + var_len);

    /* Our final string contains the prefix data, the value of the variable, the
     * remaining string size, and an extra character for the NUL byte. */
    size_t newstr_sz = var_start + strlen(value) + remain_sz + 1;

    char *newstr = malloc(sizeof(char) * newstr_sz);
    if (newstr == NULL) {
        return NULL;
    }

    strncpy(newstr, str, var_start);
    newstr[var_start] = '\0';
    strcat(newstr, value);
    strcat(newstr, str + var_start + var_len);

    return newstr;
}

/**
 * exit_shell
 * -------------------------------------------------------
 * Exits the shell whenever shell receives "exit" command
 * Closes out of shell and returns to VM as result
 * 
 * Exits out with message to user
 * Returns afterwards
 * -------------------------------------------------------
 */
void exit_shell(){
    LOGP("Exiting Shell.\n");
    return;
}

/**
 * sigint_handler
 * -----------------------------------------------------------
 * Whenever ^C is pressed by the user the shell will retrieve 
 * the message so that it does not quit out of the shell. 
 *
 * It is ignored and therefore doesn't terminate the shell. 
 * -----------------------------------------------------------
 */
void sigint_handler(int sigs){
    if(isatty(STDIN_FILENO)){               
        printf("\n");
        print_prompt();
    }
    fflush(stdout);
}

/**
 * sigchld_handler
 * -----------------------------------------------------------
 * Whenever ^C is pressed by the user the shell will retrieve 
 * the message so that it does not quit out of the shell. 
 *
 * It is ignored and therefore doesn't terminate the shell. 
 * -----------------------------------------------------------
 */
void sigchld_handler(int sigs){
    // pid_t pid = -1;
    waitpid(-1, &sigs, WNOHANG);
}


/**
 * deal_with_quotes
 * -----------------------------------------------------------
 * Check with curr_tok
 * Check with tokens
 *
 * Checks the single and double quotation marks in shell input
 * -----------------------------------------------------------
 */
void deal_with_quotes(char *copy_line){
    char *no_quotes = malloc(strlen(copy_line));
    int i, j;
    if(copy_line[0] == '"' || copy_line[0] == '\''){
        no_quotes[0] = copy_line[0];
    }
    for(i = j = 1; i < strlen(copy_line); i++){
        if((copy_line[i] == '"' && copy_line[i-1] != '\0') ||
		       	(copy_line[i] == '\'' && copy_line[i-1] != '\0')){
            continue;
	}
        no_quotes[j++] = copy_line[i];
    }
    printf("%s\n", no_quotes);
    free(no_quotes);
}

/**
 * Main function
 * ------------------------------------------------------------
 * Calls when to print out prompts from shell
 * Gets commands from line based on user input in the shell
 * Calls functions when needed to based on Linux command input
 * 
 * Keeps number of commands to shell and updates when needed
 * ------------------------------------------------------------
 */
int main(void){
    uid_t user_id = getuid();
    char *buffer = malloc(sizeof(char) * 250);
    struct passwd *pw = getpwuid(user_id);
    char *home_dir = pw->pw_dir;
    signal(SIGINT, sigint_handler);

    /**
     * Loop forever, prompting the user for commands
     */
    while (true){
    
        if(isatty(STDIN_FILENO)){               
            print_prompt();
        }
        char *line = NULL;
        size_t line_sz = 0;		
        if(getline(&line, &line_sz, stdin) == -1){
            exit(0);
        }
        // LOG("-> Got line: %s", line);
        char *copy_line = strdup(line);
        char *tokens[4096];	
        char *next_tok = line;
        char *curr_tok;
        int toksize = 0;

        /**
         * Tokenize each string word by word
         */
        while(((curr_tok = next_token(&next_tok, " \t\r\n")) != NULL) && (toksize < 4095))
        {
            
            // printf("%d\n", toksize);
            if (toksize >= 4095){
                
                tokens[toksize] = (char *) NULL;
                perror("Too many arguments.");
                break;
            }            
            if (curr_tok[0] == '#'){
                if(toksize == 0){
                    add_to_history(count, copy_line);
                    count++;
                }
                break;
            }
            if(curr_tok[0] == '$'){
                char *new_tok = expansion(curr_tok);
                if (new_tok != NULL){
                    if(curr_tok != NULL) {
                        curr_tok = new_tok;
                    }
                }
            }
            if(curr_tok[0] == '{'){
                add_to_history(count, copy_line);
                continue;
            }
            tokens[toksize++] = curr_tok;
        }   
        tokens[toksize] = (char *) NULL;
        if(tokens[0]==NULL){
            continue;
        }
        if(strcmp(tokens[0], "!!") == 0){
            get_double_bang(count);
            add_to_history(count++, copy_line);
            continue;
        }
        if(tokens[0][0] == '"' || tokens[0][0] == '\''){
            deal_with_quotes(tokens[toksize - 1]);
        }
        if(strcmp(tokens[toksize - 1], "|") == 0){
            execute_pipe(cl);
        }
        for (int i = 0; i < toksize; i++) {
            char* tmp = tokens[i];
            if(tmp[0] == '#'){
                break;
            }
        }
    
        if(strcmp(tokens[0], "history") == 0){
            //add_to_history(count, tokens[0]);
            add_to_history(count, copy_line);
            get_history();
            count++;
            continue;
            
        }
        if(strcmp(tokens[0], "setenv") == 0){
            /**
             * setenv requires two string pointers and one int
             * returns 0 if successful; -1 otherwise
             */
            if(tokens[1] != NULL && tokens[2] != NULL){
                if (setenv(tokens[1], tokens[2], 0) != -1){
                    add_to_history(count, copy_line);
                    count++;
                    continue;
                }
                else{
                    perror("setenv");
                }
            }
            
        }
        if(strcmp(tokens[0], "cd") == 0){

            if (tokens[1] == NULL){
                chdir(home_dir);
                
            } 
            else if (count > 1){
                if(chdir(tokens[1]) == -1){
                    perror("");
                }

            } 
            add_to_history(count, copy_line);
            count++;
	        free(copy_line);
            continue;
        }
        
        /**
         * Exit out of shell
         * 
         * -Calls exit_shell function
         * -Returns message to user
         * -Breaks out of while loop
         * -Exits from shell
         */
        if(strcmp(tokens[0], "exit") == 0){
            // LOGP("Exiting Shell.\n");
            exit_shell();
            exit(0);
            break;
        }
        pid_t pid = fork();
        
        if (pid == 0){
            /* Child */
            int ret = execvp(tokens[0], tokens); 
            close(fileno(stdin));
            if(ret == -1)
            {
                perror("execvp");
                break;
            }
           
        }
        else{
            /* Parent Process */
            int status;
            add_to_history(count, copy_line);
            count++; 
            waitpid(pid, &status, 0);
            signal(SIGCHLD, sigchld_handler);
            // LOG("Child exited. Status %d\n", status);
        }
        free(cl);
            
    }
    free(buffer);
}

