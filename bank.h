#ifndef BANK_H
#define BANK_H

#include <semaphore.h>

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
static void wait_for_child(int sig);
void handler(int sockfd);
void printbank();

void bank_open(char* name);
int start(char* name);
void credit(float value);
void debit(float value);
void balance();
void finish();

#endif