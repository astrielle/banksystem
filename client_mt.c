#include "bank.h"

pthread_t t_output, t_input;
char buffer[256];
int sockfd;

/*func for user input thread*/
void* input_func(){
  int n;

      printf("\nWelcome to The Bank. Please choose one of the following options:\n \"open <accountname>\",\n \"start <accountname>\",\n \"credit <amount>\",\n \"debit <amount>\",\n \"balance\",\n \"finish\",\n or \"exit\".\n\n");

  while(1){

    scanf(" %[^\n]", buffer);

    if (strcmp(buffer, "exit") == 0){
      pthread_cancel(t_output);
    }

    n = write(sockfd, buffer, 256);
    if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
    }
    sleep(2);
    memset(&buffer,0,256);
  }
  return NULL;
}

/*func for server output thread*/
void* output_func(){
  int n;
  while((n = ( read(sockfd,buffer,256))) != 0){
   // memset(&buffer,0,256);
    if(n < 0){
      perror("ERROR can't read from socket");
      exit(0);
    }
    printf("%s\n",buffer);
    memset(&buffer,0,256);
  }
  if(n == 0){
    printf("SERVER DISCONNECTED\n");
  }
  return NULL;
}

int main(int argc, char** argv){
  //socklen_t addr_size;
  struct addrinfo server_addr;
  struct addrinfo* clientinfo;
  //struct hostent *host;

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

  if((getaddrinfo(argv[1],"4999",&server_addr,&clientinfo))!=0){
    herror("ERROR invalid host\n");
    exit(0);
  }

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  //if server is not running, repeatedly tries to connect to server every 2 seconds
  while( connect(sockfd,clientinfo->ai_addr,clientinfo->ai_addrlen) < 0){

    printf("Attempting to connect to server...\n");
    sleep(3);
    
  }

  printf("Connected to server %s at port %d\n",argv[1],PORT);

  if ((pthread_create(&t_input, NULL, input_func, NULL)) != 0){
    printf("User thread creation failed\n");
    return 0;
  }

 if ((pthread_create(&t_output, NULL, output_func, argv[1])) != 0){
    printf("Server thread creation failed\n");
    return 0;
  }

  pthread_join(t_output, NULL);

  printf("Goodbye\n");

  freeaddrinfo(clientinfo);
  close(sockfd);
  return 0;
}
