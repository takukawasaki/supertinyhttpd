#include "sock.h"


int main(int ac, char *av[])
{

     if (ac != 2) {
          oops("Usage: ./timecli <IPaddress>");
     }
     int sock_id;       /* line id, file desc     */
     sock_id = connect_to_server(av[1], PORTNUM);
     if (sock_id == -1) {
          oops("connect");
     }
     talk_with_server(sock_id);
     close(sock_id);
     
}
