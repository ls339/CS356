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

  printf("host = %s\n",argv[1]);
  printf("portno = %s\n",argv[2]);
  printf("routerno = %d\n",routerno);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock < 0) { 
    printf("Could not create socket\n");
    return 0;
  }

  host = gethostbyname(argv[1]);
  if(host == NULL) {
    printf("No such host\n");
  }

  // Initialize our own table
  struct route_node *tb[N];
  initRouterTable(tb,routerno);

  // Print out our table
  route_table = printTable(tb,routerno);
  printf("%s",route_table);

  // Serialize the table
  serial_table = serializeTable(tb,routerno);
  //printf("%s",serial_table);

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;

  if(routerno!=0) {
    //bzero((char *) &serv_addr, sizeof(serv_addr));
    //serv_addr.sin_family = AF_INET;
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
    printf("TCP message received\n");
    //printf(buffer);
    printf("Sending router %d table . . .\n",routerno);
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
    //printf("%s\n",buffer);
  }
  
  close(sock);
  close(newsock);

  printf("%s\n",buffer);
  //printf("length = %d\n",strlen(buffer));
  int i,j;
  int remote_router = buffer[0] - '0';
  struct route_node *remote_tb[N];
  for(i=0;i<N;i++) {
    remote_tb[i] = malloc(sizeof(struct route_node));
  }
  printf("remote router = %d\n",remote_router);
  
  i=1;
  j=0;
  while(buffer[i] != '\0') {
    //printf("remote_tb[%d]->dest = %c\n",j,buffer[i]);
    remote_tb[j]->dest = buffer[i] - '0';
    i++;
    //printf("remote_tb[%d]->interface = %c\n",j,buffer[i]);
    remote_tb[j]->interface = buffer[i];
    i++;
    //printf("remote_tb[%d]->cost = %c\n",j,buffer[i]);
    remote_tb[j]->cost = buffer[i] - '0';
    i++;
    j++;
  }
  printf("%s",printTable2(remote_tb,remote_router));
  //int j = strlen(buffer);
  //for (i=0;i<N;i++) {
  //printf("length - 1 = %d\n",(j-1));
    //remote_tb[i-1]->dest = buffer[i] - '0';
    //remote_tb[i-1]->interface = buffer[i+1] - '0';
    //remote_tb[i-1]->cost = buffer[i+2] - '0';
  //}
  
  //  while(buffer[i] != '\0') {
  //   printf("%c",buffer[i]);
    //router_tb[i]->dest = 
  //   i++;
  // }

  return 0;
}
