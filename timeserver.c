#include "sock.h"


int main(int ac, char *av[])
{
     struct sockaddr_in cliaddr;
     socklen_t chilen;
     int sock_id, sock_fd;

     sock_id = make_server_socket(PORTNUM);
     
     printf("server accept at 8080\n");          
     while ( 1 ){

          chilen = sizeof(cliaddr);
          sock_fd = accept(sock_id, (struct sockaddr *)&cliaddr, &chilen); /* wait for call */

                   
          if ( sock_fd == -1 )
               oops( "accept" );       /* error getting calls  */
          process_request(sock_fd);
          close(sock_fd);
          
     }
     return 0;
}
