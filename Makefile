PROJECT = naziv_programa
SOURCE = main.c drugi.c treci.c
HEADERS = glavni.h pomocni.h

CC = gcc
CFLAGS = -Wall -g
LDFLAGS =
OBJECTS = ${SOURCE:.c=.o}

$(PROJECT): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROJECT)

$(OBJECTS): $(HEADERS)

clean:
	-rm -f $(PROJECT) $(OBJECTS) *.core
