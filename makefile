CC = gcc
CFLAGS = -Wall -g
COMPILE = $(CC) $(CFLAGS)

all: client server

client: client_mp.c
	$(COMPILE) —lpthread -o client client_mp.c

server: server_mp.c bank.h tokenizer.c tokenizer.h
	$(COMPILE) —lpthread -o server server_mp.c

clean:
	rm -rf *.o *.dSYM client server