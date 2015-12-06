#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define PORT 4999

pthread_t userthread;
pthread_t serverthread;
int sockfd;


void* userinput(void* arg){

  char input[256];

  printf("Welcome to The Bank.\n");
    
  while(1){
    printf("Please choose one of the following options:\n \"open <accountname>\", \"start <accountname>\", \"credit <amount>\", \"debit <amount>\", \"balance\", \"finish\", or \"exit\".\n");
    scanf(" %[^\n]", input);
    printf("You chose: %s\n", input);

    if (strcmp(input, "exit") == 0){
      pthread_cancel(serverthread);
    }

    if ((write(sockfd, input, 256)) < 0) {
      perror("ERROR writing to socket");
      exit(1);
    }
    int p = 0;
    for (; p<2; p++){
      printf("Please wait...\n");
      sleep(1);
    }
    memset(&input,0,256);
    }

    return NULL;
}

void* serveroutput(void* server){

  //char* servername=(char*)server;
  char output[1024];

  while(( read(sockfd,output,1024)) > 0){
    printf("%s", output);
  }

  return NULL;
}

int main(int argc, char** argv){
  //socklen_t addr_size;
  struct addrinfo server_addr;
  struct addrinfo* clientinfo;
  //struct hostent *host;
  int s;
  //int n;
  //pid_t pid;

  if(argc!=2){
    perror("CLIENT SIDE ERROR: Invalid number of arguments.\n");
    exit(0);
  }

  memset(&server_addr,0,sizeof(struct addrinfo));
  server_addr.ai_family = AF_INET;
  server_addr.ai_socktype = SOCK_STREAM;
  //flags set to 0 because this is client side
  server_addr.ai_flags = 0;
  server_addr.ai_protocol = 0;
  server_addr.ai_addrlen = 0;
  server_addr.ai_canonname = NULL;
  server_addr.ai_addr = NULL;
  server_addr.ai_next = NULL;

  if((s=getaddrinfo(argv[1],"4999",&server_addr,&clientinfo))!=0){
    herror("ERROR invalid host\n");
    exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  //if server is not running, repeatedly tries to connect to server every 3 seconds
  while( connect(sockfd,clientinfo->ai_addr,clientinfo->ai_addrlen) < 0){
    
    printf("Attempting to connect to server...\n");
    sleep(3);
    
  }

  printf("Connected to %s at port %d\n",argv[1],PORT);

  if ((pthread_create(&userthread, NULL, userinput, NULL)) != 0){
    printf("User thread creation failed.\n");
    return 0;
  }

 if ((pthread_create(&serverthread, NULL, serveroutput, argv[1])) != 0){
    printf("Server thread creation failed.\n");
    return 0;
  }

  pthread_join(serverthread, NULL);

  printf("Client dies\n");

  freeaddrinfo(clientinfo);
  close(sockfd);
  return 0;
}
