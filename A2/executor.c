// ------------
// This code is provided solely for the personal and private use of
// students taking the CSC369H5 course at the University of Toronto.
// Copying for purposes other than this use is expressly prohibited.
// All forms of distribution of this code, whether as given or with
// any changes, are expressly prohibited.
//
// Authors: Bogdan Simion
//
// All of the files in this directory and all subdirectories are:
// Copyright (c) 2019 Bogdan Simion
// -------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "executor.h"

struct executor tassadar;

/*
 * WARNING: This code is used for testing purposes and should not be removed or modified!
 */
 
/*
 * Prints the state of each completed_jobs queue and the resource utilization records.
 *
 */
void verify() {
    int i;
    struct job *cur;

    printf("---\n");

    /* iterate through processor records */
    for (i = 0; i < NUM_PROCESSORS; i++) {

        /* iterate through all jobs in the completed_jobs for this processor */
        for (cur = tassadar.processor_records[i].completed_jobs; cur != NULL; cur = cur->next) {
            printf("%d %d %d\n", cur->processor, cur->id, cur->type);
        }
    }

    printf("===\n");

    /* iterate through the resources and print their utilization check value */
    for (i = 0; i < NUM_RESOURCES; i++) {
        printf("%d %d\n", i, tassadar.resource_utilization_check[i]);
    }

    printf("###\n");
}

int main(int argc, char *argv[]) {
    int i;
    pthread_t admission_threads[NUM_QUEUES], execution_threads[NUM_QUEUES];

    if (argc != 2) {
        printf("Usage: %s <jobs_file>\n", argv[0]);
        exit(1);
    }

    init_executor();
    parse_jobs(argv[1]);

    /* spin up threads */
    for (i = 0; i < NUM_QUEUES; i++) {
        pthread_create(&execution_threads[i], NULL, &admit_jobs, (void *) &tassadar.admission_queues[i]);
        pthread_create(&admission_threads[i], NULL, &execute_jobs, (void *) &tassadar.admission_queues[i]);
    }

    /* wait for all execution threads */
    for (i = 0; i < NUM_QUEUES; i++) {
        if (pthread_join(execution_threads[i], NULL)) {
            exit(1);
        }
    }

    /* wait for all admission threads */
    for (i = 0; i < NUM_QUEUES; i++) {
        if (pthread_join(admission_threads[i], NULL)) {
            exit(1);
        }
    }

    verify();

    return 0;
}
