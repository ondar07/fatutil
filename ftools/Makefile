CC=gcc
CFLAGS=-c -g
LDFLAGS=
SOURCES=main.c ftools.c command_interface.c

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=fatutil

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o fatutil
