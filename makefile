CFLAGS += -Wall -Wextra -pedantic -std=c99

all: solver

solver: solver.o
	gcc $(CFLAGS) -o solver solver.o

solver.o: main.c
	gcc $(CFLAGS) -c -o solver.o main.c

clean:
	rm solver solver.o
