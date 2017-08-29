# -*- MakeFile -*-

CC = gcc
CCFLAGS = -Wall -pedantic

all: folsomprison

debug: CCFLAGS += -DDEBUG -g
debug: all

all: folsomprison

folsomprison: folsomprison.c
	$(CC) $(CCFLAGS) folsomprison.c -o folsomprison

clean:
	rm -f ./folsomprison credentials *~

