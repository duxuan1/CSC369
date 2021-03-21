#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int *t1, *t2, *b1, *b2;

int lent1, lent2, lenb1, lenb2;

int curr; // pointer of fifo stack

/* Page to evict is chosen using the ARC algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int arc_evict() {
	curr = (curr + 1) % memsize;
	return curr;
}

/* This function is called on each access to a page to update any information
 * needed by the ARC algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void arc_ref(pgtbl_entry_t *p) {

	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void arc_init() {
	lenb1 = 0;
	lenb2 = 0;
	lent1 = 0;
	lent2 = 0;
	int i;
	for (i = 0; i < memsize; i++) {
		t1[i] = 0;
		t2[i] = 0;
		b1[i] = 0;
		b2[i] = 0;
	}
	curr = -1;
}

