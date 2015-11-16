/*
 * ls339@njit.edu
 * Header file to create and manipulate router tables.
 */

#include <stdlib.h>	
#include <string.h>

#define N 4
#define INFINITY 256

struct route_node{
  int dest;
  char interface;
  int cost;
};

void initRouterTable(struct route_node **tb, int routerno) {
  int i;
  for(i=0;i<N;i++) {
    tb[i] = malloc(sizeof(struct route_node));
  }
  if(routerno==0){

    tb[0]->dest = 0;
    tb[0]->interface = 'l';
    tb[0]->cost = 0;

    tb[1]->dest = 1;
    tb[1]->interface = '0';
    tb[1]->cost = 1;

    tb[2]->dest = 2;
    tb[2]->interface = '1';
    tb[2]->cost = 3;

    tb[3]->dest = 3;
    tb[3]->interface = '2';
    tb[3]->cost = 7;
  }
  if(routerno==1){

    tb[0]->dest = 0;
    tb[0]->interface = '2';
    tb[0]->cost = 1;

    tb[1]->dest = 1;
    tb[1]->interface = 'l';
    tb[1]->cost = 0;

    tb[2]->dest = 2;
    tb[2]->interface = '0';
    tb[2]->cost = 1;

    // No direct link : set to INFINITY
    tb[3]->dest = 3;
    tb[3]->interface = 'X';
    tb[3]->cost = INFINITY;
  }
  if(routerno==2){

    tb[0]->dest = 0;
    tb[0]->interface = '2';
    tb[0]->cost = 3;

    tb[1]->dest = 1;
    tb[1]->interface = '0';
    tb[1]->cost = 1;

    tb[2]->dest = 2;
    tb[2]->interface = 'l';
    tb[2]->cost = 0;

    tb[3]->dest = 3;
    tb[3]->interface = '1';
    tb[3]->cost = 2;
  }
  if(routerno==3){

    tb[0]->dest = 0;
    tb[0]->interface = '0';
    tb[0]->cost = 7;

    // No direct link : set to INFINITY
    tb[1]->dest = 1;
    tb[1]->interface = 'X';
    tb[1]->cost = INFINITY;
    

    tb[2]->dest = 2;
    tb[2]->interface = '2';
    tb[2]->cost = 2;

    tb[3]->dest = 3;
    tb[3]->interface = 'l';
    tb[3]->cost = 0;
  }
  if(routerno==-1){
    //Intialize remote router
    for(i=0;i<N;i++){
      tb[i]->dest = i;
      tb[i]->interface = 'X';
      tb[i]->cost = INFINITY;
    }
  }
}

void updateRoute(struct route_node **tb, int dest, int cost, char iface) {
  if(tb[dest]->interface!='l') { 
    tb[dest]->cost = cost;
    tb[dest]->interface = iface;
  }
}

char * printTable(struct route_node **tb,int routerno) {
  char *str = (char *) malloc(1024);
  int i;
  sprintf(str,"+--------------------------------------------+\n");
  sprintf(str + strlen(str),"| Router %d                                   |\n",routerno);
  sprintf(str + strlen(str),"+--------------------------------------------+\n");
  sprintf(str + strlen(str),"| Destination Router | Interface | Link Cost |\n");
  sprintf(str + strlen(str),"+--------------------------------------------+\n");
  for(i=0;i<N;i++){
    if(tb[i]->cost==INFINITY) {
      sprintf(str + strlen(str),"| %18d | %9c |  Infinity |\n",tb[i]->dest,tb[i]->interface);
    } else {
      sprintf(str + strlen(str),"| %18d | %9c | %9d |\n",tb[i]->dest,tb[i]->interface,tb[i]->cost);
    }
    sprintf(str + strlen(str),"+--------------------------------------------+\n");
  }
  return str;
}

char * serializeTable(struct route_node **tb, int routerno) {
  char *str = (char *) malloc(1024);
  int i;
  sprintf(str,"%d",routerno);
  for(i=0;i<N;i++){
    if(tb[i]->cost==INFINITY) continue;
    sprintf(str + strlen(str),"%d%c%d",tb[i]->dest, tb[i]->interface,tb[i]->cost);
  } 
  return str;
}

void BellmanFord(struct route_node **local, struct route_node **remote, int remotert){
  int i;
  for(i=0;i<N;i++){
    if(local[i]->interface=='l'||remote[i]->interface=='l') continue;
    int newCost = remote[i]->cost+local[remotert]->cost;
    if(local[i]->cost > newCost) {
      printf("Updating : dest = %d to cost = %d via iface = %c\n",local[i]->dest,newCost,remote[i]->interface);
      updateRoute(local,local[i]->dest,newCost,remote[i]->interface);
    }
  }
}
