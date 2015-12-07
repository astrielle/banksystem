CC = gcc
CFLAGS = -Wall -g -lpthread
COMPILE = $(CC) $(CFLAGS)

all: client server

client: client_mt.c
	$(COMPILE) -o client client_mt.c

server: server_mp.c bank.h tokenizer.c tokenizer.h
	$(COMPILE) -o server server_mp.c

clean:
	rm -rf *.o *.dSYM client server