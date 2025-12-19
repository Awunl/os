CC = gcc
CFLAGS = -Wall -Wextra -g

all: shell

shell: main.o list.o tree.o exec.o
	$(CC) $(CFLAGS) -o shell main.o list.o tree.o exec.o
  
main.o: main.c list.h tree.h exec.h
	$(CC) $(CFLAGS) -c main.c

list.o: list.c list.h
	$(CC) $(CFLAGS) -c list.c

tree.o: tree.c tree.h list.h
	$(CC) $(CFLAGS) -c tree.c

exec.o: exec.c exec.h tree.h
	$(CC) $(CFLAGS) -c exec.c

clean:
	rm -f *.o shell

