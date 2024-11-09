CROSS_COMPILE=

CC=$(CROSS_COMPILE)gcc
CFLAGS = -g -Wall -Werror

all: aesdsocket

aesdsocket: aesdsocket.o
	$(CC) aesdsocket.o -o aesdsocket

aesdsocket.o: aesdsocket.c
	$(CC) -c aesdsocket.c 

clean:
	rm -f *.o aesdsocket
