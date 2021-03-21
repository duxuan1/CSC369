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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "executor.h"

extern struct executor tassadar;


/**
 * Populate the job lists by parsing a file where each line has
 * the following structure:
 *
 * <id> <type> <num_resources> <resource_id_0> <resource_id_1> ...
 *
 * Each job is added to the queue that corresponds with its job type.
 */
void parse_jobs(char *file_name) {
    int id;
    struct job *cur_job;
    struct admission_queue *cur_queue;
    enum job_type jtype;
    int num_resources, i;
    FILE *f = fopen(file_name, "r");

    /* parse file */
    while (fscanf(f, "%d %d %d", &id, (int*) &jtype, (int*) &num_resources) == 3) {

        /* construct job */
        cur_job = malloc(sizeof(struct job));
        cur_job->id = id;
        cur_job->type = jtype;
        cur_job->num_resources = num_resources;
        cur_job->resources = malloc(num_resources * sizeof(int));

        int resource_id; 
				for(i = 0; i < num_resources; i++) {
				    fscanf(f, "%d ", &resource_id);
				    cur_job->resources[i] = resource_id;
				    tassadar.resource_utilization_check[resource_id]++;
				}
				
				assign_processor(cur_job);

        /* append new job to head of corresponding list */
        cur_queue = &tassadar.admission_queues[jtype];
        cur_job->next = cur_queue->pending_jobs;
        cur_queue->pending_jobs = cur_job;
        cur_queue->pending_admission++;
    }

    fclose(f);
}

/*
 * Magic algorithm to assign a processor to a job.
 */
void assign_processor(struct job* job) {
    int i, proc = job->resources[0];
    for(i = 1; i < job->num_resources; i++) {
        if(proc < job->resources[i]) {
            proc = job->resources[i];
        }
    }
    job->processor = proc % NUM_PROCESSORS;
}


void do_stuff(struct job *job) {
    /* Job prints its id, its type, and its assigned processor */
    printf("%d %d %d\n", job->id, job->type, job->processor);
}



/**
 * TODO: Fill in this function
 *
 * Do all of the work required to prepare the executor
 * before any jobs start coming
 * 
 */
void init_executor() {

    // ----------initialize executor----------
    int k;
    for (k = 0; k < NUM_RESOURCES; k++) {
        pthread_mutex_init(&tassadar.resource_locks[k], NULL);
    }

    // ----------initialize admission queues----------
    int i;
    for (i = 0; i < NUM_QUEUES; i++) {
        // pthread_mutex_t lock;
        pthread_mutex_init(&tassadar.admission_queues[i].lock, NULL);
        // pthread_cond_t admission_cv, execution_cv;
        pthread_cond_init(&tassadar.admission_queues[i].admission_cv, NULL);
        pthread_cond_init(&tassadar.admission_queues[i].execution_cv, NULL);
        // list of jobs that are pending admission into this queue 
        tassadar.admission_queues[i].pending_jobs = malloc(sizeof(struct job*) * QUEUE_LENGTH);
        // number of jobs pending admission into this queue; 
        tassadar.admission_queues[i].pending_admission = 0;
        // circular list implementation storing the jobs that are admitted
        tassadar.admission_queues[i].admitted_jobs = malloc(sizeof(struct job*) * QUEUE_LENGTH);
        // maximum number of elements in the admitted_jobs list 
        tassadar.admission_queues[i].capacity = QUEUE_LENGTH;
        // number of elements currently in the admitted_jobs list 
        tassadar.admission_queues[i].num_admitted = 0;
        // index of the first element in the admitted_jobs list 
        tassadar.admission_queues[i].head = 0;
        // index of the last element in the admitted_jobs list 
        tassadar.admission_queues[i].tail = 0;
    }

    // ----------initialize processor records----------
    int j;
    for (j = 0; j < NUM_PROCESSORS; j++) {
        // some lock, use as you see fit 
        pthread_mutex_init(&tassadar.processor_records[j].lock, NULL);
        // number of jobs which have been completed on this processor
        tassadar.processor_records[j].num_completed = 0;
    }
}


/**
 * TODO: Fill in this function
 *
 * Handles an admission queue passed in through the arg (see the executor.c file). 
 * Bring jobs into this admission queue as room becomes available in it. 
 * As new jobs are added to this admission queue (and are therefore ready to be taken
 * for execution), the corresponding execute thread must become aware of this.
 * 
 */
void *admit_jobs(void *arg) {
    struct admission_queue *q = arg;
    while (1) {

        pthread_mutex_lock(&q->lock); 

        // if there is nothing left in q, exit the loop
        if ((q->pending_jobs == NULL) || (q->pending_admission <=0)){
            pthread_cond_signal(&q->admission_cv);
            pthread_mutex_unlock(&q->lock);
            return NULL; 
        }
        
        // if queue is full
        while (q->num_admitted == q->capacity) {
            pthread_cond_wait(&q->admission_cv, &q->lock);
        }

        // add the first of pending_jobs to the last of admitted_jobs
        q->admitted_jobs[q->tail] = q->pending_jobs;  

        // pop first of pending_jobs and assign to the next
        q->pending_jobs = q->pending_jobs->next;

        // number updates
        q->tail ++;
        q->tail= q->tail % q->capacity;
        q->num_admitted ++;
        q->pending_admission --;

        pthread_cond_signal(&q->admission_cv);
        pthread_mutex_unlock(&q->lock);
    }
    return NULL;
}


/**
 * TODO: Fill in this function
 *
 * Moves jobs from a single admission queue of the executor. 
 * Jobs must acquire the required resource locks before being able to execute. 
 *
 * Note: You do not need to spawn any new threads in here to simulate the processors.
 * When a job acquires all its required resources, it will execute do_stuff.
 * When do_stuff is finished, the job is considered to have completed.
 *
 * Once a job has completed, the admission thread must be notified since room
 * just became available in the queue. Be careful to record the job's completion
 * on its assigned processor and keep track of resources utilized. 
 *
 * Note: No printf statements are allowed in your final jobs.c code, 
 * other than the one from do_stuff!
 */
void *execute_jobs(void *arg) {
    struct admission_queue *q = arg;
    while (1) {

        pthread_mutex_lock(&q->lock);
        
        // if there is nothing left in q, exit the loop
        if ((q->num_admitted <=0) || (q->admitted_jobs == NULL)){
            pthread_cond_signal(&q->admission_cv);
            pthread_mutex_unlock(&q->lock);
            return NULL; 
        }

        // if if there is nothing left in admitted_jobs, wait
        while (q->num_admitted == 0) {
            pthread_cond_wait(&q->execution_cv, &q->lock);
        }

        // pop a job waiting for execute
        struct job *j = q->admitted_jobs[q->head];
        q->admitted_jobs[q->head] = NULL;

        // number updates
        q->head ++;
        q->head = q->head % q->capacity;
        q->num_admitted --;

        pthread_cond_signal(&q->execution_cv);
        pthread_mutex_unlock(&q->lock);

        // lock all needed resources, prevent other job using
        int i;
        for (i = 0; i < j->num_resources; i++) {
            pthread_mutex_lock(&tassadar.resource_locks[j->resources[i]]);
        }

        // update resource_utilization_check
        for (i = 0; i < j->num_resources; i++) {
            tassadar.resource_utilization_check[j->resources[i]] --;
        }

        // complete executing 
        assign_processor(j);
        do_stuff(j);

        // update records of processor
        // lock before assessing processor_records
        pthread_mutex_lock(&tassadar.processor_records[j->processor].lock);

        // curr pointer to pointer of linked list processor_records
        struct job **curr = &tassadar.processor_records[j->processor].completed_jobs;
        if (*curr == NULL) { // if processor_records not initialized
            *curr = j;
            (*curr)->next = NULL;
        } else { // add to the front of linkeed list
            struct job *temp = *curr;
            *curr = j;
            (*curr)->next = temp;
        }

        // number update
        tassadar.processor_records[j->processor].num_completed ++;

        // unlock after using processor_records
        pthread_mutex_unlock(&tassadar.processor_records[j->processor].lock);

        // unlock after using resources
        for (i = 0; i < j->num_resources; i++) {
            pthread_mutex_unlock(&tassadar.resource_locks[j->resources[i]]);
        } 
    }
    return NULL;
}
