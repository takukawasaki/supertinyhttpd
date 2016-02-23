#include "sock.h"


int main(int ac, char *av[])
{
     struct sockaddr_in cliaddr;
     socklen_t  chilen;
     int sock, fd;
     FILE *fpin;
     char request[BUFSIZ];

     if (ac == 1) {
          fprintf(stderr, "usage: ws portnumber\n");
          exit(1);
     }

     sock = make_server_socket(atoi(av[1]));
     if (sock == -1) {
          exit(2);
     }

     for(;;){
          chilen  = sizeof(cliaddr);
          fd = accept(sock, (struct sockaddr *)&cliaddr, &chilen);
          fpin = fdopen(fd, "r");

          fgets(request, BUFSIZ, fpin);
          printf("Got a call request: %s", request);
          process_rq(request, fd);
          fclose(fpin);
     }
}

