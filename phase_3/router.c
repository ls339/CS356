/*
 *
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

  int sock, newsock, rsock, clilen, portno, routerno, n;
  int i,j, num_dc_hosts, remote_router;
  struct hostent *host, *rhost;
  struct sockaddr_in cli_addr;
  struct sockaddr_in serv_addr,remote_addr;
  char buffer[1024];
  char *dc_hosts[N];
  char *serial_table;
  struct route_node *tb[N];
  struct route_node *remote_tb[N];

  if(argc < 4) {
    printf("Usage %s host portno routerno\n",argv[0]);
    return 0;
  }

  portno = atoi(argv[2]);
  routerno = atoi(argv[3]);
  //initRouterTable(tb,routerno);
  
  if(routerno==0) {
    num_dc_hosts = 3;
    dc_hosts[0] = "54.152.167.71";
    dc_hosts[1] = "54.86.4.199";
    dc_hosts[3] = "54.152.92.114";
  } else if(routerno==1) {
    num_dc_hosts = 2;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.152.92.114";
  } else if(routerno==2) {
    num_dc_hosts = 3;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.86.4.199";
    dc_hosts[3] = "54.152.167.71";
  }  if(routerno==3) {
    num_dc_hosts = 2;
    dc_hosts[0] = "54.164.31.62";
    dc_hosts[1] = "54.152.92.114";
  }

  enum state status;
  // printf("host = %s, port = %s\n",argv[1],argv[2]);
  initRouterTable(tb,routerno);
  serial_table = serializeTable(tb,routerno);
  status = INIT;
  
  sock = socket(AF_INET, SOCK_STREAM, 0); // Handle error
  //rsock = socket(AF_INET, SOCK_STREAM, 0);
  host = gethostbyname(argv[1]); // Handle error
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  bind(sock, (struct sockaddr *) &serv_addr,sizeof(serv_addr)); // handle error
  
  remote_addr.sin_family = AF_INET;
  remote_addr.sin_port = htons(portno);
  //listen(sock,5);
  //clilen = sizeof(cli_addr);
  //newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
  //bzero(buffer,1024);
  //n = read(newsock,buffer,1023);

  //c=getchar();
  //printf("out of loop\n");
  //listen(sock,5);
  //status = NOTUPDATED;

  printf("%s",printTable(tb,routerno));
  printf("serial table = %s\n",serial_table);
  while(1) {
    if(routerno==1 && status==INIT) {
      printf("Sending out intial table\n");
      for(i=0;i<num_dc_hosts;i++) {
	printf("--> %s\n",dc_hosts[i]);
	rsock = socket(AF_INET, SOCK_STREAM, 0);
	rhost = gethostbyname(dc_hosts[i]);
	bcopy((char *)rhost->h_addr,(char *)&remote_addr.sin_addr.s_addr,rhost->h_length);
	//remote_addr.sin_port = htons(portno);
	//connect(rsock,(struct sockaddr *)&remote_addr,sizeof(remote_addr));
	//printf("xxx\n");
	if (connect(rsock,(struct sockaddr *) &remote_addr,sizeof(remote_addr)) < 0) {
	  printf("ERROR connecting\n");
	}
	//printf("yyy\n");
	n = write(rsock,serial_table,strlen(serial_table));
	//printf("%s",printTable(tb,routerno));
	printf("eol\n");
	close(rsock);
      }
    }
      //bcopy((char *)host->h_addr,(char *)&remote_addr.sin_addr.s_addr,host->h_length);
      //remote_addr.sin_port = htons(portno);
      //connect(sock,(struct sockaddr *)&remote_addr,sizeof(remote_addr));
      //n = write(sock,serial_table,strlen(serial_table));
    listen(sock,5);
    clilen = sizeof(cli_addr);
    newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
    bzero(buffer,1024);
    n = read(newsock,buffer,1023);
    printf("in loop\n");
    //remote_router = buffer[0] - '0';
    //initRouterTable(remote_tb,-1);

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
      //printf("%s",printTable(tb,i));
      j++;
    }
    
    //printf("remote router = %d\n",remote_router);
    //if( (BellmanFord2(tb,remote_tb,remote_router)) == 1) {
    //	status = UPDATED;
    //  } else {
    //	status = NOTUPDATED;
    //  }
    //printf("%s",buffer);
    //printf("%s",printTable(tb,routerno));
    status = BellmanFord2(tb,remote_tb,remote_router);
    printf("status = %d \n",status);
    if(status == UPDATED) {
      printf("%s",printTable(tb,routerno));
    }
  }
  
  
  return 0;
}
