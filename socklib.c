#include "sock.h"

int make_server_socket(int portnum)
{
     return make_server_socket_q(portnum,BACKLOG);
}

int make_server_socket_q(int portnum, int backlog)
{
     struct sockaddr_in saddr, cliaddr;
     struct hostent *hp;
     char hostname[HOSTLEN];
     int sock_id;

     sock_id = socket(AF_INET, SOCK_STREAM, 0);
     if (sock_id == -1) {
          return -1;
     }

     bzero((void *)&saddr, sizeof(saddr));
     gethostname(hostname, HOSTLEN);
     hp = gethostbyname(hostname);
     saddr.sin_addr.s_addr = htonl(INADDR_ANY);
     saddr.sin_port = htons(portnum);
     saddr.sin_family = AF_INET;

     if (bind(sock_id, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
          return -1;
     }

     if (listen(sock_id, backlog) != 0) {
          return -1;
          
     }
     return sock_id;
}


int connect_to_server(char *host, int portnum)
{
     int sock;
     struct sockaddr_in saddr;
     struct hostent *hp;

     sock = socket(AF_INET, SOCK_STREAM, 0);
     if (sock == -1) {
          return -1;
          
     }
     bzero(&saddr, sizeof(saddr));
     hp = gethostbyname(host);
     if (hp == NULL) {
          return -1;
     }

     bcopy(hp->h_addr, (struct sockaddr *)&saddr.sin_addr, hp->h_length );
     saddr.sin_port = htons(portnum);
     saddr.sin_family = AF_INET;

     if (connect (sock, (struct sockaddr *)&saddr, sizeof(saddr)) != 0) {
          return -1;
     }

     return sock;
     
}

void process_request(int fd)
{
     int pid = fork();
     switch (pid) {
     case -1: {
          return;
          break;
     }
     case 0: {
          dup2(fd,1);
          close(fd);
          execl("/bin/date","date",NULL);
          oops("execlp");
          break;
     }
     default:
          wait(NULL);
          break;
     }
}

void talk_with_server(int fd)
{
     char buf[BUFSIZ];
     int n;
     n = read(fd,buf,BUFSIZ);
     write(1,buf,n);
     
}
void
read_til_crnl(FILE *fp)
{
     char buf[BUFSIZ];
     while (fgets(buf, BUFSIZ, fp) != NULL && strcmp(buf, "\r\n") != 0) {
          ;
     }
     
}

void
process_rq( char *rq, int fd )
{
     char cmd[BUFSIZ], arg[BUFSIZ];

     /* create a new process and return if not the child */
     if ( fork() != 0 )
          return;

     strcpy(arg, "./");/* precede args with ./ */
     if ( sscanf(rq, "%s%s", cmd, arg+2) != 2 )
          return;

     if ( strcmp(cmd,"GET") != 0 )
          cannot_do(fd);
     else if ( not_exist( arg ) )
          do_404(arg, fd );
     else if ( isadir( arg ) )
          do_ls( arg, fd );
     else if ( ends_in_cgi( arg ) )
          do_exec( arg, fd );
     else
          do_cat( arg, fd );
     
}


/* ------------------------------------------------------ *
   the reply header thing: all functions need one
   if content_type is NULL then don't send content type
   ------------------------------------------------------ */
void
header( FILE *fp, char *content_type )
{
     fprintf(fp, "HTTP/1.0 200 OK\r\n");
     if ( content_type )
          fprintf(fp, "Content-type: %s\r\n", content_type );
     
}


/* ------------------------------------------------------ *
   simple functions first:
        cannot_do(fd)       unimplemented HTTP command
    and do_404(item,fd)     no such object
    ------------------------------------------------------ */

void
cannot_do(int fd)
{
     FILE *fp = fdopen(fd,"w");

     fprintf(fp, "HTTP/1.0 501 Not Implemented\r\n");
     fprintf(fp, "Content-type: text/plain\r\n");
     fprintf(fp, "\r\n");

     fprintf(fp, "That command is not yet implemented\r\n");
     
}

void
do_404(char *item, int fd)
{
     FILE*fp = fdopen(fd,"w");

     fprintf(fp, "HTTP/1.0 404 Not Found\r\n");
     fprintf(fp, "Content-type: text/plain\r\n");
     fprintf(fp, "\r\n");

     fprintf(fp, "The item you requested: %s\r\nis not found\r\n",
             item);
     fclose(fp);
     
}


/* ------------------------------------------------------ *
   the directory listing section
   isadir() uses stat, not_exist() uses stat
   do_ls runs ls. It should not
   ------------------------------------------------------ */
int isadir(char *f)
{
     
     struct stat info;
     return ( stat(f, &info) != -1 && S_ISDIR(info.st_mode) );
}

int
not_exist(char *f)
{
     struct stat info;
     return( stat(f,&info) == -1 );
     
}

int
do_ls(char *dir, int fd)
{
     FILE*fp ;

     fp = fdopen(fd,"w");
     header(fp, "text/plain");
     fprintf(fp,"\r\n");
     fflush(fp);

     dup2(fd,1);
     dup2(fd,2);
     close(fd);
     execlp("ls","ls","-l",dir,NULL);
     perror(dir);
     exit(1);
     
}



/* ------------------------------------------------------ *
   the cgi stuff.  function to check extension and
   one to run the program.
   ------------------------------------------------------ */

char * file_type(char *f)
/* returns 'extension' of file */
{
     char*cp;
     if ( (cp = strrchr(f, '.' )) != NULL )
          return cp+1;
     return "";
     
}

int
ends_in_cgi(char *f)
{
     return ( strcmp( file_type(f), "cgi" ) == 0 );
     
}

void
do_exec( char *prog, int fd )
{
     FILE*fp ;

     fp = fdopen(fd,"w");
     header(fp, NULL);
     fflush(fp);
     dup2(fd, 1);
     dup2(fd, 2);
     close(fd);
     execl(prog,prog,NULL);
     perror(prog);
}

/* ------------------------------------------------------ *
   do_cat(filename,fd)
   sends back contents after a header
   ------------------------------------------------------ */

int
do_cat(char *f, int fd)
{
     char *extension = file_type(f);
     char *content = "text/plain";
     FILE *fpsock, *fpfile;
     int c;

     if ( strcmp(extension,"html") == 0 )
          content = "text/html";
     else if ( strcmp(extension, "gif") == 0 )
          content = "image/gif";
     else if ( strcmp(extension, "jpg") == 0 )
          content = "image/jpeg";
     else if ( strcmp(extension, "jpeg") == 0 )
          content = "image/jpeg";

     fpsock = fdopen(fd, "w");
     fpfile = fopen( f , "r");
     if ( fpsock != NULL && fpfile != NULL )
     {
          header( fpsock, content );
          fprintf(fpsock, "\r\n");
          while( (c = getc(fpfile) ) != EOF )
               putc(c, fpsock);
          fclose(fpfile);
          fclose(fpsock);
          
     }
     exit(0);
     
}
