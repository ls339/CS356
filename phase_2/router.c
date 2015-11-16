/*
 * ls339@njit.edu
 *
 */
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "routerTable.h"

int main(int argc,char **argv) {

  int sock, newsock, clilen, portno, routerno, n;
  struct hostent *host;
  struct sockaddr_in cli_addr;
  struct sockaddr_in serv_addr;
  char *route_table, *serial_table;
  char buffer[1024];

  if(argc < 4) {
    printf("Usage %s host portno routerno\n",argv[0]);
    return 0;
  }

  portno = atoi(argv[2]);
  routerno = atoi(argv[3]);

  printf("host = %s, port = %s\n",argv[1],argv[2]);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) { 
    printf("Could not create socket\n");
    return 0;
  }

  host = gethostbyname(argv[1]);
  if(host == NULL) {
    printf("No such host\n");
  }

  // Initialize local table
  struct route_node *tb[N];
  initRouterTable(tb,routerno);

  // Print out local table
  route_table = printTable(tb,routerno);
  printf("%s",route_table);

  // Serialize the local table
  serial_table = serializeTable(tb,routerno);

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  /*
   * Action based on router number input,
   * router = 0 is designated a client.
   * router != 0 are servers.
   */
  if(routerno!=0) {
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
      error("ERROR on binding");
    }
    listen(sock,5);
    clilen = sizeof(cli_addr);
    newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
    if (newsock < 0) error("ERROR on accept");
    bzero(buffer,1024);
    n = read(newsock,buffer,1023);
    if (n < 0) error("ERROR reading from socket");
    printf("Sending router %d table . . .\n",routerno);
    printf("TCP message received\n");
    n = write(newsock,serial_table,strlen(serial_table));
    if (n < 0) error("ERROR writing to socket");
  } else {
    bcopy((char *)host->h_addr,(char *)&serv_addr.sin_addr.s_addr,host->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
      error("ERROR connecting");
    printf("Sending router %d table . . .\n",routerno);
    n = write(sock,serial_table,strlen(serial_table));
    printf("TCP message received\n");
    n = write(sock,buffer,strlen(buffer));
    if (n < 0)
      error("ERROR writing to socket");
    bzero(buffer,1024);
    n = read(sock,buffer,1023);
    if (n < 0)
      error("ERROR reading from socket");
  }
  
  close(sock);
  close(newsock);

  /*
   * Take received serialized data
   * and convert it to route_node struct.
   */
  int i,j;
  int remote_router = buffer[0] - '0';
  struct route_node *remote_tb[N];
  initRouterTable(remote_tb,-1);

  i=1;
  j=0;
  while(buffer[i] != '\0') {
    remote_tb[j]->dest = buffer[i] - '0';
    i++;
    remote_tb[j]->interface = buffer[i];
    i++;
    remote_tb[j]->cost = buffer[i] - '0';
    i++;
    j++;
  }
  
  // Print remote table
  printf("%s",printTable(remote_tb,remote_router));
  // Calculate new table 
  BellmanFord(tb,remote_tb,remote_router);
  // Print new table
  printf("%s",printTable(tb,routerno));

  return 0;
}
