#ifndef _HISTORY_H_
#define _HISTORY_H_

#define HIST_MAX 100

struct history_entry {
    unsigned long cmd_id;
    double run_time;
    char *command;
};
struct history_entry cmd_struct[HIST_MAX];
long count; 

void get_history();
void get_specific_bang(int num);
void get_range_bang(int num);
void get_double_bang(int num);
void reallocate();
void add_to_history(int count, char *line);
void run_last_cmd(char *tokens);

#endif
