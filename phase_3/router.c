/*
 *
 * ls339@njit.edu
 * Phase 3
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include "routerTable.h"

int main(int argc,char **argv) {

  int sock, newsock, rsock, clilen, portno, routerno, n;
  int i,j, num_dc_hosts, remote_router;
  struct hostent *host, *rhost;
  struct sockaddr_in cli_addr, serv_addr, remote_addr;
  char buffer[1024];
  char *dc_hosts[N];
  char *serial_table;
  struct route_node *tb[N];
  struct route_node *remote_tb[N];
  enum state status;

  if(argc < 3) {
    printf("Usage %s portno routerno\n",argv[0]);
    return 0;
  }

  portno = atoi(argv[1]);
  routerno = atoi(argv[2]);

  /*
   * Hard coded aws routers.
   */  
  if(routerno==0) {
    num_dc_hosts = 3;
    dc_hosts[0] = "54.152.167.71";
    dc_hosts[1] = "54.86.4.199";
    dc_hosts[2] = "54.152.92.114";
  } else if(routerno==1) {
    num_dc_hosts = 2;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.152.92.114";
  } else if(routerno==2) {
    num_dc_hosts = 3;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.86.4.199";
    dc_hosts[2] = "54.152.167.71";
  }  if(routerno==3) {
    num_dc_hosts = 2;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.152.92.114";
  }

  initRouterTable(tb,routerno);
  serial_table = serializeTable(tb,routerno);
  status = INIT;
  
  sock = socket(AF_INET, SOCK_STREAM, 0); // Handle error
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  bind(sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)); // handle error
  
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(portno);

  printf("Initial table:\n%s",printTable(tb,routerno));
  
  while(1) {
    if(routerno==1 && status==INIT) {
      printf("Sending out intial table.\n");
      for(i=0;i<num_dc_hosts;i++) {
	printf("connecting to %s . . .\n",dc_hosts[i]);
	rsock = socket(AF_INET, SOCK_STREAM, 0);
	rhost = gethostbyname(dc_hosts[i]);
	bcopy((char *)rhost->h_addr,(char *)&remote_addr.sin_addr.s_addr,rhost->h_length);
	if (connect(rsock,(struct sockaddr *) &remote_addr,sizeof(remote_addr)) < 0) {
	  printf("ERROR connecting\n");
	}
	n = write(rsock,serial_table,strlen(serial_table));
	close(rsock);
      }
    }
    listen(sock,5);
    clilen = sizeof(cli_addr);
    newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
    bzero(buffer,1024);
    n = read(newsock,buffer,1023);

    /*
     * Take received serialized data
     * and convert it to route_node struct.
     */
    remote_router = buffer[0] - '0';
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
    
    status = BellmanFord2(tb,remote_tb,remote_router);

    if(status == UPDATED) {
      printf("Updates recieved\nUpdated table:\n%s",printTable(tb,routerno));
      printf("Sending updated table to connected links\n");
      serial_table = serializeTable(tb,routerno);
      for(i=0;i<num_dc_hosts;i++) {
	printf("Connecting to %s . . .\n",dc_hosts[i]);
	rsock = socket(AF_INET, SOCK_STREAM, 0);
        rhost = gethostbyname(dc_hosts[i]);
        bcopy((char *)rhost->h_addr,(char *)&remote_addr.sin_addr.s_addr,rhost->h_length);
	if (connect(rsock,(struct sockaddr *) &remote_addr,sizeof(remote_addr)) < 0) {
          printf("ERROR connecting\n");
        }
	n = write(rsock,serial_table,strlen(serial_table));
	close(rsock);
      }
    }
  }
  return 0;
}
