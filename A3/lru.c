#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int curr;

int *timestamp_map;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
	int index = 0;
	int min;
	int i;
	for (i = 0; i < memsize; i++) {
		if (timestamp_map[i] < min) {
			index = i;
			min = timestamp_map[i];
		}
	}
	return index;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
	int frame = p->frame >> PAGE_SHIFT;
	timestamp_map[frame] = curr++;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
	curr = 0;
	// init reference map to all 0s
	timestamp_map = malloc(sizeof(int) * memsize);
}
