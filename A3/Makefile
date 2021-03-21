
CFLAGS=-std=gnu99 -Wall -g

sim :  sim.o pagetable.o swap.o rand.o clock.o lru.o fifo.o arc.o
	gcc $(CFLAGS) -o sim $^

%.o : %.c pagetable.h sim.h
	gcc $(CFLAGS) -g -c $<

clean : 
	rm -f *.o sim *~
