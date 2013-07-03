CC = gcc
CFLAGS = -Wall -g 
LDFLAGS =

HEADERS = pomocni.h

POMOCNI_C = pomocni.c
POMOCNI_O = ${POMOCNI_C:.c=.o} 

SERVER_C = udpserver.c
SERVER_O = ${SERVER_C:.c=.o}

KLIJENT_C = udpklijent.c
KLIJENT_O = ${KLIJENT_C:.c=.o}

all: udpserver udpklijent

udpserver: $(SERVER_O) $(POMOCNI_O)
	$(CC) $(CFLAGS) $(SERVER_O) $(POMOCNI_O) -o $@

udpklijent: $(KLIJENT_O) $(POMOCNI_O)
	$(CC) $(CFLAGS) $(KLIJENT_O) $(POMOCNI_O) -o $@

$(SERVER_O): $(HEADERS)
$(KLIJENT_O): $(HEADERS)
$(POMOCNI_O): $(HEADERS)

clean:
	-rm -f udpserver udpklijent *.o *.core
