#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int curr; // pointer of clock

int *reference_map; // reference for each clock sections

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	while (1) {
		curr %= memsize; // clock can not be bigger than memsize
		if (reference_map[curr] == 0) {
			return curr;
		} else { // second chance
			reference_map[curr] = 0;
		}
		curr ++;
	}
	return 0;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	int frame = p->frame >> PAGE_SHIFT;
	reference_map[frame] = 1;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	curr = 0;
	// init reference map to all 0s
	reference_map = malloc(sizeof(int) * memsize);
}
