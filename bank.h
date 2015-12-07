#ifndef BANK_H
#define BANK_H

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
#include <time.h>

#include <sys/ipc.h>
#include <sys/shm.h> 
#include <semaphore.h>

#define PORT 4999

 typedef struct account_{
   char name [100];
   float balance;
   int insession;
   sem_t mutex;
 }account;
 
 typedef struct database_{
   account accountlist[20];
   int count;
   sem_t mutex;
 }database;

int search_db(database* db, char* name);
char* lowerstring(char* str);
void printbank();

void wait_for_child(int sig);

void bank_open(char* name);
int start(char* name);
void credit(float value);
void debit(float value);
void balance();
void finish();

void clientcommands(int newfd);

#endif
