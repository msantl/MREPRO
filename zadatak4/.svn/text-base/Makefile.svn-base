PROJECT = tftpserver
SOURCE = pomocni.c tftpserver.c
HEADERS = pomocni.h

CC = gcc
CFLAGS = -Wall -g
LDFLAGS = 
OBJECTS = ${SOURCE:.c=.o}

$(PROJECT): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(PROJECT)

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
