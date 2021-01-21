#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"

char *next_token(char **str_ptr, const char *delim);
char def_wp[] = "HTTP/1.1 200 OK\r\n"
                "Date: %s\r\n"
                "Content-Length: %zu\r\n"
                "%s\r\nGrandma says 404\r\n";

/**
 * Generates an HTTP 1.1 compliant timestamp for use in HTTP responses.
 *
 * Inputs:
 *  - timestamp: character pointer to a string buffer to be filled with the
 *    timestamp.
 */
void generate_timestamp(char *timestamp)
{
    time_t now = time(0);
    struct tm time = *gmtime(&now);
    strftime(timestamp, sizeof(timestamp), "%a, %d %b %Y %H:%M:%S %Z", &time);
}

/**
 * Reads from a file descriptor until:
 *  - the newline ('\n') character is encountered
 *  - *length* is exceeded
 *  This is helpful for reading HTTP headers line by line.
 *
 * Inputs:
 *  - fd: file descriptor to read from
 *  - buf: buffer to store data read from *fd*
 *  - length: maximum capacity of the buffer
 *
 * Returns:
 *  - Number of bytes read;
 *  - -1 on read failure
 *  - 0 on EOF
 */
ssize_t read_line(int fd, char *buf, size_t length)
{
    ssize_t total = 0;
	while (total < length) {
		ssize_t read_sz = read(fd, buf + total, length - total);
		if (*(buf + total) == '\n') {
			return total + read_sz;
		}
		if (read_sz == -1) {
			perror("read");
			return -1;
		} else if (read_sz == 0) {
			return 0;
		}

		total += read_sz;
	}
		
	return total;
}

/**
 * File not found function is here
 */
void not_found(int fd) {
    char msg[8192] = {0};
    char date[8192] = {0};
    char* error = "Grandma says 404 go away";
    generate_timestamp(date);
    // sprintf(msg, "HTTP/1.1 404 Not Found\r\n"
    //                 "Date: %s\r\n"
    //                 "Content-Length: %zu\r\n"
    //                 "\r\n"
    //                 "Grandma says 404\r\n",
    //                 date, strlen(error), error);

    printf("Message: %s\n", msg);
    sprintf("HTTP/1.1 404 Not Found\r\n"
                    "Date: %s\r\n"
                    "Content-Length: %zu\r\n"
                    "\r\n"
                    "Grandma says 404\r\n",
                    date, strlen(error), error);
    LOG("Sending responses:\n%s", msg);
    write(fd, msg, strlen(msg));

}



/**
 * TODO: reads an HTTP 1.1 request and responds with the appropriate file (or
 * 404 if the file does not exist).
 */
int handle_request(int fd)
{
	LOGP("Handling request\n");
	char path[8192] = { 0 };

	while (true){
    	char header[8192] = {0};
		char uri[8192] = { 0 };

		read_line(fd, header, 8192);
		LOG("-> %s", header);
		char* next_tok = header;
		char* curr_tok;

		curr_tok = next_token((char **)next_tok, " \t\r\n");
		if(curr_tok != NULL && strcmp(curr_tok, "GET") == 0){
			curr_tok = next_token((char **)next_tok, " \t\r\n");
			uri[0] = '.';
			strcpy(&uri[1], curr_tok);
			LOG("URI: %s\n", curr_tok); 
		} else if(curr_tok == NULL){
			break;
		}
	}
	LOG("File path: %s\n", path);
	struct stat sb;
	int ret = stat(path, &sb);
	if(ret == -1){
		perror("stat");
	}	// send a 404
		
	char msg[8192] = {0};
	char date[8192] = {0};
	generate_timestamp(date);
	// sprintf(msg, "HTTP/1.1 200 OK\r\n"
	// 			"Date: %s\r\n"
	// 			"Content-Length: %zu\r\n"
	// 			"%s\r\nGrandma says 404\r\n",
	// 			date, sb.st_size);

    sprintf("HTTP/1.1 200 OK\r\n"
                "Date: %s\r\n"
                "Content-Length: %zu\r\n"
                "%s\r\nGrandma says 404\r\n",
                date, sb.st_size);
	LOG("Sending responses:\n%s", msg);

    write(fd, msg, strlen(msg));

	int file_fd = open(path, O_RDONLY);
	// check error code
	//sendfile(fd, ret, file_fd, sb.st_size, header);

	//sendfile(int fd, int s, off_t offset, off_t *len, struct sf_hdtr *hdtr, int flags);
    sendfile(fd, 0, 0, sb.st_size); 
	return 0;
}

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}


int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s port dir\n", argv[0]);
        return 1;
    }

    int new_sock_fd, port_num, client_len;
    char buffer[2048];
    int pid;
    struct sockaddr_in client_addr, addr = {0};
    int port = atoi(argv[1]);
    char *dir = argv[2];

	LOG("Starting; using port %d\n", port);

    /* TODO:
     *  - Create server socket
     *  - Bind to the specified port
     *  - Listen for incoming connections
     */
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd < 0){
		perror("ERROR socket_fd");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if(bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1){
		perror("ERROR bind");
        close(socket_fd);
		exit(1);
	}
    /*
     * Start listening for clients
     * Process to wait for incoming connection
     */
	if(listen(socket_fd, 10) == -1){
		perror("listen");
		close(socket_fd);
        exit(1);
	}

    LOG("Listening on port %d\n", port);
    client_len = sizeof(struct sockaddr_in);
    
    /*
        Check the path to see if valid and that the path does exist
    */
	LOG("chdir to %s\n", dir);
	int ret = chdir(dir);
	if(ret == -1){
		perror("chdir");
        return 1;
	}
	
	while(true) {
        new_sock_fd = 
        accept(socket_fd, (struct sockaddr_in *) &client_addr, (socklen_t *)&client_len);

        if (new_sock_fd == -1) {
            perror("Connection failed - ERROR accept");
            continue;
        }
        LOG("Got client connection %d\n", new_sock_fd);

        int pid = fork();
        if (pid < 0) {
            // Child Process
            close(socket_fd);
            memset(new_sock_fd, buffer, 2047);
            LOG("%s\n", buffer);

            if(!strncmp(buffer, "GET /index.html", 15)) {
                int new_fd = open("index.html", O_RDONLY);
                sendfile(new_sock_fd, new_fd, NULL, 4000);
                close(new_fd);
            }  else if(!strncmp(buffer, "GET tests/html/syllabus/", 15)) {
                int new_fd = open("tests/html/syllabus/index.html", O_RDONLY);
                sendfile(new_sock_fd, new_fd, NULL, 6000);
                close(new_fd);
            }
            else {
                write(new_sock_fd, def_wp, sizeof(def_wp) - 1);
            }
            close(new_sock_fd);
            printf("Closing.\n");
            exit(0);
            
        }
        // Parent Process
        close(new_sock_fd);

	}

    

    return 0; 
}
