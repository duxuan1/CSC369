#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define RECORD_SIZE 128  

struct krec {
	double d[RECORD_SIZE];
};

void stack_loop(int iters) {
	int i;
	struct krec a[iters];
	for(i = 0; i < iters; i++) {
		a[i].d[0] = (double)i;
	}
	(void)a; /* Use a to keep compiler happy */
}

int main(int argc, char ** argv) {
	/* Markers used to bound trace regions of interest */
	volatile char MARKER_START, MARKER_END;
	/* Record marker addresses */
	FILE* marker_fp = fopen("stack.marker","w");
	if(marker_fp == NULL ) {
		perror("Couldn't open marker file:");
		exit(1);
	}
	fprintf(marker_fp, "%p %p", &MARKER_START, &MARKER_END );
	fclose(marker_fp);

	MARKER_START = 33;
	stack_loop(100);
	MARKER_END = 34;

	return 0;
}
