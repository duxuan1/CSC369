all: executor

CFLAGS=-std=gnu99 -Wall -g

executor: executor.o jobs.o
	gcc $(CFLAGS) -pthread -o $@ $^

%.o : %.c executor.h
	gcc $(CFLAGS) -c $<

clean : 
	rm -f *.o executor *~ results.txt

