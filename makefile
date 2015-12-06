CC = gcc
CFLAGS = -Wall -g
COMPILE = $(CC) $(CFLAGS)

all: client server

client: client_mp.c
	$(COMPILE) -o client client_mp.c

server: server_mp.c bank.h tokenizer.c tokenizer.h
	$(COMPILE) -o server server_mp.c

clean:
	rm -rf *.o *.dSYM client server