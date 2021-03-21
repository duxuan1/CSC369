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
#ifndef __EXECUTOR_H__
#define __EXECUTOR_H__

#include <pthread.h>

#define NUM_QUEUES 4
#define QUEUE_LENGTH 10
#define NUM_PROCESSORS 4
#define NUM_RESOURCES 8


/* job types */
enum job_type {
    INTERACTIVE,
    REALTIME,
    BATCH,
    UNKNOWN,
    NUM_TYPES
};

/* Representation of a job being launched */
struct job {
    /* id that uniquely identifies each job */
    int id;

    /* job type */
    enum job_type type;

    /* array of required resource numbers. 
       each number is between [0, NUM_RESOURCES-1] */
    int *resources;
    
    /* assigned processor */
    int processor;
    
    /* number of required resources, must be at least 1. */
    int num_resources;

    /* this link is to support implementing a list of jobs */
    struct job *next;
};

/* admission queue */
struct admission_queue {
    /* synchronization */
    pthread_mutex_t lock;
    pthread_cond_t admission_cv, execution_cv;

    /* list of jobs that are pending admission into this queue */
    struct job *pending_jobs;

    /* number of jobs pending admission into this queue; */
    int pending_admission;

    /* circular list implementation storing the jobs that are admitted */
    struct job **admitted_jobs;

    /* maximum number of elements in the admitted_jobs list */
    int capacity;

    /* number of elements currently in the admitted_jobs list */
    int num_admitted;

    /* index of the first element in the admitted_jobs list */
    int head;

    /* index of the last element in the admitted_jobs list */
    int tail;
};


/* processor record, tracking which jobs completed on it */
struct processor_record {
	
    /*
     * list of jobs which have completed execution on this processor.
     * This list should only be appended to and left in memory for us
     * to use during testing. Please do not free the list or any of the jobs.
     */
    struct job *completed_jobs;

    /* number of jobs which have been completed on this processor. */
    int num_completed;

    /* some lock, use as you see fit ;) */
    pthread_mutex_t lock;
};


/* executor */
struct executor {
    /* Each resource lock ensures only one job can hold the resource at any given moment. */
    pthread_mutex_t resource_locks[NUM_RESOURCES];

    /* Executor keeps track of how many times a resource should be acquired and decrements
       the corresponding element when the corresponding resource has been acquired. */
    int resource_utilization_check[NUM_RESOURCES];

    /* The admission queues */
    struct admission_queue admission_queues[NUM_QUEUES];    

    /* Processor bookkeeping info */
    struct processor_record processor_records[NUM_PROCESSORS];    
};

/* forward declaration of logic functions */
void parse_jobs(char *f_name);
void init_executor();

void *admit_jobs(void *arg);
void *execute_jobs(void *arg);

void assign_processor(struct job* job);

#endif
