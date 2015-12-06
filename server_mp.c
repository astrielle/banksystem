#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define PORT 4999

#include "bank.h"
#include "tokenizer.c"

//not sure where this goes, but each client process needs its own copy so probably not here
int session_active = -1;

//global vars
key_t key;
int shmid;
int sockfd, newfd;
char str[256];

database* db;

//helper function returns index of specified account
int search_db(database* db, char* name){
	int i = 0;
	for(;i<20;i++){
		if(strcmp(db->accountlist[i].name,name)==0){
			return i;
		}
	}
	return -1;
}

//converts a string to lowercase; helper method for command comps
char* lowerstring(char* str){
	int i;
	for (i=0; i < strlen(str); i++){
		str[i] = tolower(str[i]);
	}
	return str;
}

void sigint_handler(int signo){
  if (signo == SIGINT)
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}

//prints entire contents of bank
void printbank(){

	//**New accounts cannot be opened while the bank is printing out the account information.**

	if (db->count < 1){
		printf("The bank has no accounts.\n");
	}

	int i = 0;
	for (;i<db->count;i++){
		printf("Account name: %s, Account balance: %.2f ", db->accountlist[i].name, db->accountlist[i].balance);
		if (db->accountlist[i].insession == 1){
			printf("IN SERVICE\n");
		}
		else printf("\n");
	}
}

//opens a new bank account
void bank_open(char* name){

	//wait on bank to unlock from printing if necessary
	//sem_wait(&db->mutex);

	//max 20 acounts
	if(db->count>=20){
		write(newfd, "Open error: The bank is full and cannot accept new accounts.\n", 256);
		return;
	}
	//can't open account while in a session
	if (session_active >=0){
		write(newfd, "Open error: Cannot open new account while account session is active.\n", 256);
		return;
	}
	//must have different names
	if(search_db(db,name)!=-1){
		write(newfd, "Open error: An account with that name already exists.\n", 256);
		return;
	}
	//init account
	if(db->count<20){
		int num = db->count;
		strcpy(db->accountlist[num].name, name);
		db->accountlist[num].balance = 0;
		db->accountlist[num].insession = 0;
		db->count++;
		//sem_init(&(db->accountlist[num].mutex), 1, 1);
		sprintf(str, "Successfully opened account %s.\n", name);
		write(newfd, str, 256);
		printf("Account counter: %d\n", db->count);
	}
}

//starts an account session
int start(char* name){
	//no concurrent sessions on client process
	if (session_active >= 0){
		write(newfd, "Start error: There is another active account session on this process.\n", 256);
		exit(1);
	}

	//find index of account
	int i = search_db(db, name);
	printf("account %s is at index %d\n", name, i);

	//account name not found
	if (i < 0 || i > 19){
		write(newfd, "Start error: There is no account with that name.\n", 256);
		exit(1);
	}

	//account is already in session on another client process
	
	if (db->accountlist[i].insession == 1){
	write(newfd, "Start error: There is already an active session for that account. Concurrent sessions are not allowed.\n", 256);
		exit(1);
	}

/*
	while((sem_trywait(&db->accountlist[i].mutex)) != 0){
		write(newfd, "Account is already in session. Trying again...", 256);
		sleep(2);
	}
*/
	db->accountlist[i].insession = 1;

	sprintf(str, "Successfully started session for %s.\n", name);
	write(newfd, str, 256);
	return i;
}

//adds money to account
void credit(float value){
	//can only run when account is in session
	if (db->accountlist[session_active].insession < 0){
		write(newfd, "Credit error: An account is not in session. Please start an account session first.\n", 256);
		exit(1);
	}
	db->accountlist[session_active].balance += value;
		sprintf(str,"Successful credit of %.2f. Your new balance is %.2f.\n", value, db->accountlist[session_active].balance);
		write(newfd, str, 256);
	return;
}

//removes money from account, cannot go below 0
void debit(float value){
	//can only run when account is in session
	if (db->accountlist[session_active].insession < 0){
		write(newfd, "Debit error: An account is not in session. Please start an account session first.\n", 256);
		exit(1);
	}
	if (value > db->accountlist[session_active].balance){
		sprintf(str, "Debit error: You do not have enough funds. Your current balance is %.2f.\n", db->accountlist[session_active].balance);
		write(newfd, str, 256);
		return;
	}

	db->accountlist[session_active].balance -= value;
	sprintf(str,"Successful debit of %.2f. Your new balance is %.2f.\n", value, db->accountlist[session_active].balance);
	write(newfd, str, 256);
	return;
}

//prints balance
void balance(){
	//can only run when account is in session
	if (db->accountlist[session_active].insession < 0){
		write(newfd, "Balance error: An account is not in session. Please start an account session.\n", 256);
		exit(1);
	}
	sprintf(str,"Your current balance is: %.2f.\n", db->accountlist[session_active].balance);
	write(newfd, str, 256);
	return;
}

//finishes an account session
void finish(){
	//can only run when account is in session
	if (session_active < 0){
		write(newfd, "Finish error: There is no active account session to finish.\n", 256);
		return;
	}

	sprintf(str,"Finishing session for %s.\n", db->accountlist[session_active].name);
	write(newfd, str, 256);

	//sem_post(&db->accountlist[session_active].mutex);
	db->accountlist[session_active].insession = 0;
	session_active = -1;
	return;
}

/* Signal handler to reap zombie processes */
static void wait_for_child(int sig)
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handler(int sockfd){
	printf(" *HANDLER: CHILD PROCESS %d (my parent is %d) \n", getpid(), getppid());
  //does this time have to match the client connection time?
  // sleep(5);

}

int main(int argc, char** argv){

	/*VARIABLES*/
	struct addrinfo *servinfo;    
	struct addrinfo hints;      
	socklen_t addr_size;
	struct sockaddr_in client_addr;

	int s;
	//int n; 
	int yes = 1;

	char buffer[256];

	pid_t pid;
	struct sigaction sa;	

	/*ERROR CHECK: ARGS*/
	if(argc!=1){
		printf("SERVER ERROR: Invalid number of arguments.\n");
		return 0;
	}

	memset(&hints,0,sizeof(struct addrinfo));  

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
  	//because this is the server, will be wildcard IP ( 0.0.0.0)
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	if( (s = getaddrinfo(NULL, "4999", &hints, &servinfo)) != 0){
		printf("error converting hostname to address\n");
		exit(1);
	}

	if( (sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))<0 ){
		perror("ERROR socket");
		exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("ERROR setsockopt");
		exit(1);
	} 

	if( bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen)<0 ){
		perror("ERROR bind");
		exit(1);
	}

  //backlog has maxsize of 20
	if(listen(sockfd,20)== -1){
		perror("ERROR listen");
		exit(1);
	}

	freeaddrinfo(servinfo);

	printf("Listen success: Waiting for client connections...\n");

  	/* Set up the signal handler */
	sa.sa_handler = wait_for_child;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	/*set up sigint handling*/
	signal(SIGINT, sigint_handler);

	/* CREATE SHARED MEMORY*/
	key = ftok("server", 'x');
	shmid = shmget(key, sizeof(database), 0666 | IPC_CREAT);

	//Make a new socket on server side for each accepted client connection, accept 1 connection from queue each time
	addr_size = sizeof(struct sockaddr_in);
	//int i=0;
	int counter=0;

	printf("Waiting for connections...\n");

	while(1){
		if(  (newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size)) == -1 ){
			perror("ERROR socket accept");
			exit(1);
		}
		counter++;
		printf("Counter: %d\n", counter);

			db = shmat(shmid, (void*)0, 0);
			if (db == (database*)(-1)){
				printf("ERROR shared memory no data returned\n");
				exit(1);
			}
			db->count = 0;
			//sem_init(&db->mutex, 1, 1);

    	//else
		printf("--FORK--\n");
		pid = fork();

		//child process
		if(pid == 0){
			printf("CHILD PROCESS %d (my parent is %d) \n", getpid(), getppid());

			memset(&buffer,0,256);  

			/*

			if((n = ( read(newfd,buffer,256))) < 0){
				perror("ERROR can't read from socket");
				exit(0);
			}

			printf("child CLIENT %d SAYS: %s\n",counter, buffer);

			n = write(newfd,"HAIL SATAN'S CHILDREN",256);

			if (n < 0) {
				perror("ERROR writing to socket");
				exit(1);
			}
			*/

			/*CLIENT COMMANDS*/

			while((read(newfd, buffer, 256)) > 0){

				TokenizerT* tk = NULL;
				char* token = NULL;
				//float value = 0;
				tk = TKCreate(buffer);

				//printf("INPUT: %s, LENGTH: %lu\n", tk->str, strlen(tk->str));

				while((token = TKGetNextToken(tk)) != NULL){

					lowerstring(token);

					//printf("input: %s\n", buffer);

					if (strcmp(token, "open") == 0){
	      			//next token is acct name
						printf("OPEN\n");
						if ((token = TKGetNextToken(tk)) == NULL){

							write(newfd, "Please specify an account name.\n", 256);
							continue;
						}
						if (strlen(token) > 100){
							write(newfd, "The account name must be shorter than 100 characters.\n", 256);
							continue;
						}

						bank_open(token);
					}
					else if (strcmp(token, "start") == 0){
			      	//next token is acct name
			      	//returns index of started session
						printf("START\n");
						session_active = start(TKGetNextToken(tk));
						printf("Session activated: %d\n", session_active);

					}
					else if (strcmp(token, "credit") == 0){
			      	//next token is amount - convert to float
						printf("CREDIT\n");
						credit(atof(TKGetNextToken(tk)));

					}
					else if (strcmp(token, "debit") == 0){
						printf("DEBIT\n");
			      	//next token is amount - convert to float
						debit(atof(TKGetNextToken(tk)));

					}
					else if (strcmp(token, "balance") == 0){
						printf("BALANCE\n");
						balance();

					}
					else if (strcmp(token, "finish") == 0){
						printf("FINISH\n");
						finish();

					}
					else if(strcmp(token, "exit") == 0){
						finish();
						break;
					}
					else{
						write(newfd, "That is not a valid client command.\n", 256);
						continue;
					}
				}
				TKDestroy(tk);
			}

			//end client commands
			
			if ((shmdt(db)) == -1){
				perror("ERROR shared memory detach failed\n");
			}
			
			close(newfd);
			//exit(0);
		}

		//parent process
		else if(pid>0){
			printf("PARENT PROCESS %d!\n", getpid());

			memset(&buffer,0,256);

			//print bank every 20 seconds
			while(1){
				sleep(20);
				//sem_wait(&db->mutex);
				printbank();
				//sem_post(&db->mutex);
			}

			/*

			if((n = ( read(newfd,buffer,256))) < 0){
				perror("ERROR can't read from socket");
				exit(0);
			}

			*/

			//printf("parent CLIENT %d SAYS: %s\n",counter, buffer);

			/*
			n = write(newfd,"HAIL SATAN",256);

			if (n < 0) {
				perror("ERROR writing to socket");
				exit(1);
			}
			*/

			//printf("parent buffer: %s\n", buffer);

			//close(sockfd);
			//printf("PARENT CLOSES NEWFD\n");

		}
		else{
			perror("ERROR can't fork");
			exit(1);
		}
	}

  /* int status;
     pid_t p;
     while (counter > 0) {
     p = wait(&status);
     printf("Child with PID %ld exited with status 0x%x.\n", (long)p, status);
     --counter;  
     printf("counter at wait: %d\n");
     }
  */

  //Never reaches this point because still looping??
    printf("Server dies\n");

    sockfd = -1;
    return 0;
 }
