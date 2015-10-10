/*
 * ls339@njit.edu
 * CS356
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char **argv) {

  int sock, port;
  char buffer[256];

  /*
   * int socket(int domain, int type, int protocol);
   * AF_INET : IPv4 Internet protocols
   * SOCK_STREAM : Provides sequenced, reliable, two-way, connection-based byte streams.  
   * An out-of-band data transmission mechanism may be supported.
   * Protocol : The protocol specifies a particular protocol to be used with the socket.  
   * Normally only a single protocol exists to support a particular socket type within a given 
   * protocol family, in which case protocol can be specified as 0.
   */
  sock = socket(AF_INET, SOCK_STREAM, 0);

  port = 35000;
  
  printf("TCP Port : %d\n",port);
}
