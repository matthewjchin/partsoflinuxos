#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Preprocessor Directives */
#ifndef DEBUG
#define DEBUG 1
#endif
/**
 * Logging functionality. Set DEBUG to 1 to enable logging, 0 to disable.
 */
#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


/* Function prototypes */
void print_usage(char *argv[]);
void getSysInfo(char *);
void getHardwareInfo(char *);
void getTaskInfo(char *);
void getTasks(char *);
char *next_token(char **, const char *);
// char* readline(int, char*, char*);
bool readline(int, char*);
bool digitDir(char *);

void getPIDTasks(char*);

/* This struct is a collection of booleans that controls whether or not the
 * various sections of the output are enabled. */
struct view_opts
{
    bool hardware;
    bool system;
    bool task_list;
    bool task_summary;
};

void print_usage(char *argv[])
{
    printf("Usage: %s [-ahlrst] [-p procfs_dir]\n" , argv[0]);
    printf("\n");
    printf("Options:\n"
"    * -a              Display all (equivalent to -lrst, default)\n"
"    * -h              Help/usage information\n"
"    * -l              Task List\n"
"    * -p procfs_dir   Change the expected procfs mount point (default: /proc)\n"
"    * -r              Hardware Information\n"
"    * -s              System Information\n"
"    * -t              Task Information\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    
    /* Default location of the proc file system */
    char *procfs_loc = malloc(sizeof(char) * 150);
    procfs_loc = "/proc";
    chdir("/proc");

    /* Set to true if we are using a non-default proc location */
    bool alt_proc = false;

    struct view_opts all_on = { true, true, true, true };
    struct view_opts options = { false, false, false, false };

    int c;
    opterr = 0;
    while ((c = getopt(argc, argv, "ahlp:rst")) != -1)
    {
        switch (c) {
            case 'a': // Default - similart to -lrst, displays all
                options = all_on;
		getTasks(procfs_loc);
		getHardwareInfo(procfs_loc);
		getSysInfo(procfs_loc);
		getTaskInfo(procfs_loc);
                break;
            case 'h':  
                print_usage(argv);
		return 0;
            case 'l': // Task list
                options.task_list = true;
                getTasks(procfs_loc);
                break;
            case 'p':
            // Change the expected procfs mount point (default: /proc)
            // Preferred procfs location
                procfs_loc = optarg;
                if(chdir(procfs_loc) != 0)
		{
			perror("No such file or directory");
			return -1;
		}
		alt_proc = true;
                break;
            case 'r': // Hardware Information
                options.hardware = true;
		getHardwareInfo(procfs_loc);		
                break;
            case 's': // System Information
                options.system = true;	
		getSysInfo(procfs_loc);
                break;
            case 't': // Task Information
                options.task_summary = true;
                getTaskInfo(procfs_loc);

                break;
            case '?':
                if (optopt == 'p')
                {
                    fprintf(stderr,
                            "Option -%c requires an argument.\n", 
                            optopt);
                }
                else if (isprint(optopt))
                {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n", 
                            optopt);
                }
                print_usage(argv);
                return 1;
            default:
                abort();
        }
    }

    if (alt_proc == true)
    {
        LOG("Using alternative proc directory: %s\n", procfs_loc);

        /* Remove two arguments from the count: one for -p, one for the
         * directory passed in: */
        argc = argc - 2;
    }

    if (argc <= 1)
    {
        /* No args (or -p only). Enable all options: */
        options = all_on;
    }

    LOG("Options selected: %s%s%s%s\n",
            options.hardware ? "hardware " : "",
            options.system ? "system " : "",
            options.task_list ? "task_list " : "",
            options.task_summary ? "task_summary" : "");
    
    return 0;
}

// Hardware Information - r
void getHardwareInfo(char* sys_dir)
{
    char* file_hard = "cpuinfo";
    char * buffer = calloc(250, sizeof(char));
    
    // off_t set = 0;
    // size_t lnct = 0;
    // int x=0;

    int fd = open(file_hard, O_RDONLY); // file descriptor
    printf("Hardware Information\n"
            "---------------------\n");
    
    // get the CPU model name
    printf("CPU Model: ");
      
    ssize_t read_sz;
    char *next_tok = buffer;
    // char *curr_tok;
    char *to_search = malloc(sizeof(char) * 250);
    to_search = ":";

    char* cpu_model = malloc(sizeof(char) * 250);
    // bool isit = true;
    
    char *arrstr = malloc(sizeof(char) * 250);
    // printf("%ld\n", read_sz);
    
    // int line = 0;
    int pos = 0;
    char * line;
    while((read_sz = read(fd, buffer, 250) > 0)){   
        // int newstr = atoi(arrstr);
        
        // printf("%ld", read_sz);
        while((*arrstr = next_token(&next_tok, to_search) != NULL)){
            
            // // printf
            // // arrstr = readline(fd, buffer);
            // newstr = readline(fd, buffer);
            // printf("%ld\n", newstr);
            // if(readline(fd, buffer) == false){
            //     break;
            // }
            line = arrstr;
            printf("%s", line);
            if(line[pos] == '\n'){
                line[pos] = '\0';
                break;
            }
            else{ 
                pos++; 
            }

        } 
        // line = arrstr;
        // printf("%s", line);
        // if(line[pos] == '\n'){
        //     line[pos] = '\0';
        //     break;
        // }
        // else{ pos++; }
    }
    printf("%s\n", buffer);
    printf("%s\n", arrstr);
    // printf("%s\n", curr_tok);


    // curr_tok = next_token(&next_tok, to_search);

    printf("\nProcessing Units: ");
    char *buffer2 = malloc(sizeof(char) * 250);
    char *c2 = buffer2;
    
    // char *c1;
    int units=1;
    ssize_t r2;

    while((r2 = read(fd, c2, 100) > 0)){
        // while((c1 = readline(&c2, to_search)) != false)
        
        while(readline(*c2, to_search) != true)
        {
            units++;
            if(strstr(c2, "siblings") != NULL){
                units++;
            }
        }
    }

    printf("%d\n", units);
    
    
    char *dir_load = malloc(sizeof(char) * 100);
    // buffer2 = calloc(250, sizeof(char));

    char * buffer3 = "loadavg";
    char *lavg = malloc(sizeof(char) * 100);
//    int lad = open(dir_load, O_RDONLY); // file descriptor
    int lad = open(buffer3, O_RDONLY);
    
    printf("Load Average (1/5/15 min): ");
    ssize_t read_sz2;
    char* num = malloc(sizeof(char) * 50);
    int y=0;
    while ((read_sz2 = read(lad, lavg, 1)) > 0)
    {
        if(lavg[0] != ' ')
        {
            float dec;
            num[0] = *lavg;
            dec = atof(num);
            printf("%.2f ", dec);
            y++;
            if(y == 3)
            {
                break;
            }
        }
    }
    
    // retrieve thru stat file in /proc
    printf("\nCPU Usage: ");
    float pctcpu = 0, pctmem = 0, gb = 0;
    char* cpuline = malloc(sizeof(char) * 100);
    cpuline = "--------------------";
    printf("[%s] %.1f%%", cpuline, pctcpu);

    
    // retrieve thru meminfo file in /proc
    printf("\nMemory Usage: ");
    char* memline = malloc(sizeof(char) * 100);
    memline = "--------------------";
    // (0.0 GB / 1.0 GB)
    printf("[%s] %.1f%% (%.1f GB / 1.0 GB)", memline, pctmem, gb);

    printf("\n");
    free(buffer);
    free(buffer2);
    
    // free(buffer3);
    
    free(next_tok);
    // free(curr_tok);
    
    // free(c1);
    free(c2);
    
    free(cpu_model);
    // free(file_hard);
    free(dir_load);

    free(lavg);

    // free allocated memory for CPU Model name
    free(arrstr);

    // free allocated memory for CPU usage
    // free(cpuline);
    // free(memline);

    // free allocated memory for Mem usage


    free(num);
    
    close(fd);
    close(lad);
    printf("\n");
}

// System Information - s
void getSysInfo(char* sys_dir)
{
    
    printf("\nSystem Information\n"
           "--------------------\n");
    
    // Retrieve Hostname of Virtual Machine
    printf("Hostname: ");
    // char *file_host = malloc(sizeof(char) * 100);
    // strcpy(file_host, "sys/kernel/hostname");
    char *file_host = "sys/kernel/hostname";
    char *b1 = malloc(sizeof(char) * 100);
    int cd1 = open(file_host, O_RDONLY);

    ssize_t read_sz;
    while ((read_sz = read(cd1, b1, 1)) > 0)
    {
        int i;
        for (i = 0; i < read_sz; ++i) {
            if (b1[i] != '\0'){
                printf("%c", b1[i]);
            }
        }
    }

    // Retrieve Kernel Version
    printf("Kernel Version: ");
    char *file_kernel = malloc(sizeof(char) * 100);
    strcpy(file_kernel, "sys/kernel/osrelease");
    char *b2 = malloc(sizeof(char) * 100);
    int cd2 = open(file_kernel, O_RDONLY);
    
    ssize_t read_k;
    while((read_k = read(cd2, b2, 1) > 0))
    {
        for (int j = 0; j < read_k; ++j)
        {
            if (b2[j] != '\0'){
                printf("%c", b2[j]);
            }
        }
    }
    
    // Retrieve uptime of VM
    printf("Uptime: ");
    char *file_ut = malloc(sizeof(char) * 100);
//    strcpy(file_ut, "/proc/uptime");
    strcpy(file_ut, "uptime");
    char *b3 = malloc(sizeof(char) * 100);
    int cd3 = open(file_ut, O_RDONLY);
    
    // Get the individual time values
    int year=0, day=0, hour=0, min=0; //, sec=0;
    double seconds;
    int k=0;
    char * number = malloc(sizeof(char) * 100);
    ssize_t read_ut;
    while((read_ut = read(cd3, b3, 1) > 0))
    {
        if(b3[0] != ' ')
        {
            number[k] = *b3;
            k++;
        }
        if(b3[0] == ' ')
        {
            break;
        }
    }
    seconds = atof(number);
    
    long s = (long)seconds;
    int ry=0, rd=0, rh=0, rm=0, rs=0;
    year = (int)s/31536000;
    ry += (s % 31536000);
    day = ry/86400;
    rd += (ry%86400);
    hour = rd/3600;
    rh += (rd % 3600);
    min = rh / 60;
    rm += (rh % 60);
    // sec = rm;
    rs += rm;
    
    
    //years
    if(year != 0)
    {
        printf("%d years, ", year);
    }


    //days
    if(day > 0)
    {
        printf("%d days, ", day);
    }

    // hours
    if(hour > 0)
    {
        printf("%d hours, ", hour);
    }

    
    // minutes
    if(min > 0)
    {
        printf("%d minutes, ", min);
    }

    // seconds
    if(rs > 0)
    {
//        rs = (rm%60);
//        printf("%d seconds\n", rs);
        printf("%d seconds\n", rs);
    }
    if(rs == 0)
    {
        rs = (rm%60);
    }

    printf("\n");

    /*
        Free:
        -Directory Path Memory for hostname
        -Buffer Memory
        Close:
        -File Descriptor Memory
    */
    // free(file_host);
    free(b1);
    close(cd1);

    /*
        Free:
        -Directory Path Memory for kernel version
        -Buffer Memory
        Close:
        -File Descriptor Memory
    */
    free(file_kernel);
    free(b2);
    close(cd2);

    /*
        Free:
        -Directory Path Memory for uptime
        -Buffer Memory
        Close:
        -File Descriptor Memory
    */
    free(file_ut);
    free(b3);
    close(cd3);
 
}

// task list information - t
void getTaskInfo(char* sys_dir)
{

    printf("\nTask Information:\n"
    "-----------------------\n");
    
    long num_tasks = 1;
    // task count - count the number of PIDs running
    printf("Task(s) Running: "); 

    DIR *d;
    int x=0;
    d = opendir(".");
    struct dirent * entry;

    // while((d = readdir(entry)) != NULL){

    //     if(isdigit(entry->d_name[x]) == 1)
    //     {
    //         // printf("%d", entry->d_name[x]);
    //         num_tasks++; 
    //         x++;
    //     }
    //     if(isalpha(entry->d_name[x]) == 1)
    //     // if(isdigit(entry->d_name[x]) == 0) 
    //     {
    //         // printf("%d", entry->d_name[x]);
    //         num_tasks--;
    //         x++;
    //     }
    //     // x++;

    //     //     else break;
    //     // }
    // }
    closedir(d);

    printf("%ld", num_tasks);

    printf("\nSince boot: \n"
    "\tInterrupts: \n"
    "\tContext Switches: \n"
    "\tForks: \n");

    printf("\n");
}

bool digitDir(char *str){
    int i;
    size_t len = strlen(str);
    for(i=0; i<len; i++)
    {
        if(!(isdigit(str[i])))
        {
            return false;
        }
    }
    return true;

}

// Retrieve tasks running - l
void getTasks(char* sys_dir)
{

    printf("%5s | %12s | %25s | %15s | %s \n"
    "------+--------------+---------------------"
    "------+-----------------+-----------\n",
           "PID", "State", "Task Name", "User", "Tasks");
    sys_dir = "stat";
    getPIDTasks(sys_dir);

    
    // char *b1 = malloc(sizeof(char) * 100);
    // int cd1 = open(file_host, O_RDONLY);

    // ssize_t read_sz;
    // while ((read_sz = read(cd1, b1, 1)) > 0)
    // {
    //     int i;
    //     for (i = 0; i < read_sz; ++i) {
    //         if (b1[i] != '\0'){
    //             printf("%c", b1[i]);
    //         }
    //     }
    // }

    // printf("%5s | %12s | %25s | %15s | %s \n",
    //        "PID", "State", "Task Name", "User", "Tasks");

    printf("\n");
}

void getPIDTasks(char *dir)
{
    char *b1 = malloc(sizeof(char) * 100);
    // int cd1 = open(file_host, O_RDONLY);
    int cd1 = open(dir, O_RDONLY);

    ssize_t read_sz;
    while ((read_sz = read(cd1, b1, 1)) > 0)
    {
        for (int i = 0; i < read_sz; ++i) {
            if (b1[i] != '\0'){
                printf("%c", b1[i]);
            }
        }
    }
    free(b1);
    close(cd1);
}

char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL)
    {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) 
    {
        *str_ptr = NULL;
        return NULL;
    }


    char *current_ptr = *str_ptr + tok_start;
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {

        *str_ptr = NULL;
    } else {

        **str_ptr = '\0';
        (*str_ptr)++;
    }

    return current_ptr;
}

// char* readline(int fd, char* path, char* buffer){

// Check line by line for any new line character and cut off there
// Return true or false statement
bool readline(int fd, char* buffer){
    // ssize_t read_sz;
    // char *next_tok = malloc(sizeof(char) * 100);
    char *curr_tok = buffer;
    int i = 0;
    // char *line_str = malloc(sizeof(char) * 250);
    char c;
    bool eoline = false;

    while((c = read(fd, &c, 100)) > 0)
    {
        if(!(c == '\n')){
            c = '\0';
            eoline = true;
        }
        else{
            // c = NULL;
            c++;
            curr_tok[i] = c;
            i++;
            eoline = false;
            break;
        }
        
    
    // if(curr_tok[0] == '\n'){
    // if(curr_tok[0] != '\n')
    //     if(curr_tok[0] != ' ')
    //     {
    //         // printf("%c", c);
    //         if(!(c == '\n')){

    //             int i = strlen(curr_tok) - 1;
    //             next_tok[i] = '\0';
    //             return true;
    //         }
    //     }
    //     else
    //     {
    //         return true;         
    //     }
        
    //     return curr_tok;
    // }
    // char * n = next_tok;
    // free(next_tok);
    // free(curr_tok);
    // return false;

        free(curr_tok);
        // return eoline;
    }
    return eoline;

}



 
