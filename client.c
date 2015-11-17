#include <stdio.h>
#include <stdlib.h>

typedef struct _account{
	char[100] accountname;
	float balance;
	int insession;
} account;

typedef struct _database{
	account[20] accountlist;
	int counter;
} database;

typedef enum command{
	open,
	start,
	credit,
	debit,
	balance,
	finish,
	userexit
}COMMAND;

database* clients = malloc(sizeof(database));
clients->counter = 0;

void open(char* accountname){
	if (clients->counter == 20){
		printf("Sorry, the client base is full. New accounts cannot be created at this time.\n");
		return;
	}
	else{
		account* newuser = malloc(sizeof(account));
		newuser->accountname = accountname;
		newuser->balance = 0;
		newuser->insession = 0;
		clients->accountlist[counter] = newuser;
		clients->counter++;
	}
	return;
}

int start(char* accountname){
	int i;
	for(i = 0; i < clients->counter; i++){
		if (strcmp((clients->accountlist[i]->accountname), accountname) == 0){
			if(clients->accountlist[i]->insession == 1){
				printf("This user's session is already started.\n");
				return -1;
			}
		clients->accountlist[i]->insession = 1;
		printf("Starting session for %s.\n", accountname);
		return(i);
		}
	}
	else{
		printf("No account found for account name %s.\n", accountname);
		return -1;
	}
}

void credit(int currentuser, float amount){
	
}

void debit(int currentuser, float amount){
	
}

void balance(int currentuser){
	printf("Your current balance is: %.2f\n", clients->accountlist[currentuser]->balance);
	return;
}

char* finish(int currentuser){
	clients->accountlist[currentuser]->insession = 0;
	printf("Finishing session for %s.\n", accountname);
	return NULL;
}

void userexit(){

}

int main(int argc, char**argv){
	char* response = NULL;
	char* accountname[100];
	int currentuser = -1;

	//I/O loop
	printf("Welcome to the Client Side Bank. Please choose one of the following options:\n \"open <accountname>\", \"start <accountname>\", \"credit <amount>\", \"debit <amount>\", \"balance\", \"finish\", or \"exit\".\n");
	scanf("%s", &response);



	switch(response){
		case "open":
			if (currentuser != -1){
				printf("A user session is currently running. Please finish the user session before opening a new account.\n");
				break;
			}
			open(accountname);
		case "start": 
			currentuser = start(accountname);
		case "credit":
			if (currentuser == -1){
				printf("A user session is not currently running. Please start a user session.\n");
				break;
			}
			credit(currentuser, amount);
		case "debit":
			if (currentuser == -1){
				printf("A user session is not currently running. Please start a user session.\n");
				break;
			}
			debit(currentuser, amount);
		case "balance":
			if (currentuser == -1){
				printf("A user session is not currently running. Please start a user session.\n");
				break;
			}
			balance(currentuser);
		case "finish":
			if (currentuser == -1){
				printf("A user session is not currently running. Please start a user session.\n");
				break;
			}
			finish(currentuser);
		case "exit":
			userexit();
		default:
			printf("I'm sorry, I don't recognize that command. Please enter,
				\"open\", \"start\", \"credit\", \"debit\", \"balance\", \"finish\", or \"exit\":\n");
	}

	return 0;
}