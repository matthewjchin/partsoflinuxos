#include <stdio.h>
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
#include <time.h>

// Checks to see if there is debugging needed at any point in program - LOG and LOGP returning
//#include "debug.h"  
// Retrieves the history of all commands given to shell and corresponding pid number
#include "history.h"  
// Gets the time taken that shell is on and/or used to take time
#include "timer.h" 
// Tokenizes each string into memory word by word
#include "tokenizer.h" 

int head = 0, tail = 0;
bool full = false;

/**
 * get_history
 * -----------------------------------------------
 * Keep track of: 
 *      -Count of the number of indices in array
 *      -The array of structs called cmd_struct
 *      -Size of array is HIST_MAX
 * -----------------------------------------------
 */
void get_history(){
    // printf("history\n");
    if(full == true){ 
        
        int i;
        for (i = head; i < ((HIST_MAX) + head); i++){
            // printf("%ld  %.2f  %s", 
                // cmd_struct[i % HIST_MAX].cmd_id, 
                // cmd_struct[i % HIST_MAX].run_time, cmd_struct[i % HIST_MAX].command);
        // }    
	           
            printf("%ld  %s", cmd_struct[i % HIST_MAX].cmd_id, cmd_struct[i % HIST_MAX].command);
        }
    }
    else{
        for (int j = 0; j < tail; j++){
            // printf("%ld  %.2f  %s", 
            //     cmd_struct[j%HIST_MAX].cmd_id, 
            //     cmd_struct[j%HIST_MAX].run_time, cmd_struct[j%HIST_MAX].command);
        //}
         
            printf("%ld  %s", cmd_struct[j%HIST_MAX].cmd_id, cmd_struct[j%HIST_MAX].command);
        }
    }
}

/**
 * get_double_bang
 * ------------------------------------------
 * Function called if "!!" is found
 * 
 * Should execute that command in addition 
 * to returning the command that the user 
 * put into shell, if that command is valid.
 * ------------------------------------------
 */
void get_double_bang(int num){
    char *cmd;
    for(int i=0; i<(HIST_MAX + head); i++){
        if (i == num){
            cmd = cmd_struct[num].command;
            printf("%s", cmd);
            free(cmd);
            break;
        }
    }
}

/**
 * get_range_bang
 * ---------------------------------------------------------------
 * Checks to see if the number that the user put in is valid and 
 * if so returns the last x number of commands as put in by the 
 * user, including the last command. 
 * ---------------------------------------------------------------
 */
void get_range_bang(int num){
    if(num == -1){
	    perror("bang");
    }
    else{
        int stop = count - num;
        for(int i=num; i<stop; i++){
            // printf("%ld  %.2f  %s", 
            // cmd_struct[num].cmd_id, cmd_struct[num].run_time, cmd_struct[num].command);
            printf("%ld  %s", cmd_struct[i].cmd_id, cmd_struct[i].command);
        }
            
    }
}

/**
 * add_to_history
 * -----------------------------------------------------------------------
 * Adds command count index and command to the struct
 * Arranges space if the command count is 100 or more to return the last 
 * 100 commands entered into the shell
 * -----------------------------------------------------------------------
 */
void add_to_history(int count, char *line){
    
    if(tail == HIST_MAX-1){
        full = true;
    }
    clock_t time;
    time = clock();
    cmd_struct[tail].cmd_id = count;
    time = clock() - time;
    cmd_struct[tail].run_time = time;
    cmd_struct[tail].command = line;
    tail = (tail + 1) % HIST_MAX;
    if(full == true){
        head = tail;
    }
    count++;
}

