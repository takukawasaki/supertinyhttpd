#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#include <strings.h>
#include <netdb.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORTNUM 8080
#define HOSTLEN 256
#define BACKLOG  1
#define oops(m) {perror(m); exit(1);}

int make_server_socket(int );
int make_server_socket_q(int, int);
int connect_to_server(char *, int);

void process_request(int);
void talk_with_server(int);
void read_til_crnl(FILE *);
void process_rq(char *, int fd);
void header(FILE *fp, char *);
void cannot_do(int );
void do_404(char *, int );
int isadir(char *);
int not_exist(char *);
int do_ls(char *, int);
char * file_type(char *);
int ends_in_cgi(char *);
void do_exec(char *, int);
int do_cat(char *, int);


