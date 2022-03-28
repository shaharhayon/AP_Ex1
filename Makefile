CC=gcc
AR=ar
FLAGS= -Wall -g

all: slast

slast: slast.o
	$(CC) $(FLAGS) -o slast slast.o

slast.o: slast.c
	$(CC) $(FLAGS) -c slast.c

.PHONY: clean
clean:
	rm -f slast *.o *.a