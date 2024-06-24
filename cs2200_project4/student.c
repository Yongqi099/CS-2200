/*
 * student.c
 * Multithreaded OS Simulation for CS 2200
 * Spring 2023
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "student.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

static unsigned int cpu_count;

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 *
 * rq is a pointer to a struct you should use for your ready queue
 * implementation. The head of the queue corresponds to the process
 * that is about to be scheduled onto the CPU, and the tail is for
 * convenience in the enqueue function. See student.h for the
 * relevant function and struct declarations.
 *
 * Similar to current[], rq is accessed by multiple threads,
 * so you will need to use a mutex to protect it. ready_mutex has been
 * provided for that purpose.
 *
 * The condition variable queue_not_empty has been provided for you
 * to use in conditional waits and signals.
 *
 * Please look up documentation on how to properly use pthread_mutex_t
 * and pthread_cond_t.
 *
 * A scheduler_algorithm variable and sched_algorithm_t enum have also been
 * supplied to you to keep track of your scheduler's current scheduling
 * algorithm. You should update this variable according to the program's
 * command-line arguments. Read student.h for the definitions of this type.
 */
static pcb_t **current;
static queue_t *rq;

static pthread_mutex_t current_mutex;
static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;

static sched_algorithm_t scheduler_algorithm;
static unsigned int cpu_count;
static unsigned int age_weight;

// keep track of timeslice length
static int timeslice = -1;

/** ------------------------Problem 3-----------------------------------
 * Checkout PDF Section 5 for this problem
 * 
 * priority_with_age() is a helper function to calculate the priority of a process
 * taking into consideration the age of the process.
 * 
 * It is determined by the formula:
 * Priority With Age = Priority - (Current Time - Enqueue Time) / Age Threshold
 * 
 * @param current_time current time of the simulation
 * @param process process that we need to calculate the priority with age
 * 
 */
extern double priority_with_age(unsigned int current_time, pcb_t *process) {
    double base_priority = process->priority;
    double age = current_time - process->enqueue_time;
    double priority_with_age = base_priority + age * (double) (age_weight);
    return priority_with_age;
}

/** ------------------------Problem 0 & 3-----------------------------------
 * Checkout PDF Section 2 and 5 for this problem
 * 
 * enqueue() is a helper function to add a process to the ready queue.
 * 
 * NOTE: For Priority scheduling, you will need to have additional logic
 * in this function and/or the dequeue function to account for enqueue time
 * and age to pick the process with the smallest age priority.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 *
 * @param queue pointer to the ready queue
 * @param process process that we need to put in the ready queue
 */
void enqueue(queue_t *queue, pcb_t *process) {
    // Lock the mutex to protect the shared data
    pthread_mutex_lock(&queue_mutex);

    // If the queue is empty
    if (is_empty(queue)) {
        // Add the process as the only element in the queue
        queue->head = queue->tail = process;
    } else {
        // Add the process to the end of the queue
        queue->tail->next = process;
        queue->tail = queue->tail->next;
        queue->tail->next = NULL;
    }
    process->enqueue_time = get_current_time();
    // Signal that the queue is not empty
    pthread_cond_signal(&queue_not_empty);
    // Unlock the mutex
    pthread_mutex_unlock(&queue_mutex);
}

/**
 * dequeue() is a helper function to remove a process to the ready queue.
 *
 * NOTE: For Priority scheduling, you will need to have additional logic
 * in this function and/or the enqueue function to account for enqueue time
 * and age to pick the process with the smallest age priority.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 * 
 * @param queue pointer to the ready queue
 */

pcb_t *dequeue(queue_t *queue) {
    // Lock the mutex to protect the shared data
    pthread_mutex_lock(&queue_mutex);
    // Declare a variable to hold the process to be dequeued
    pcb_t *proc;

    // Wait until the queue is not empty
    while (is_empty(queue)) {
        pthread_mutex_unlock(&queue_mutex);
        return 0;
        // pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }

    double cur_time = get_current_time();

    // If the scheduling algorithm is Priority Aging (PA)
    if (scheduler_algorithm == PA) {
        // Find the process with the highest priority in the queue
        pcb_t *prev = NULL; // Pointer to the process before the one with the highest priority
        pcb_t *cur = queue->head; // Pointer to the current process being checked
        double max_priority = priority_with_age(cur_time, cur); // Maximum priority found so far

        while (cur->next) { // Loop until the end of the queue is reached
            double next_proc_priority = priority_with_age(cur_time, cur->next); // Priority of the next process in the queue
            if (next_proc_priority > max_priority) { // If the next process has a higher priority than the maximum found so far
                prev = cur; // Update prev to point to the current process
                max_priority = next_proc_priority; // Update max_priority to the priority of the next process
            }
            cur = cur->next; // Move to the next process in the queue
        }

        // Remove the found process from the queue
        if (prev) { // If a process was found before the one with the highest priority
            proc = prev->next; // Set process to point to the process with the highest priority
            prev->next = prev->next->next; // Remove the process with the highest priority from the queue
            if (!prev->next) { // If prev is now at the end of the queue
                queue->tail = prev; // Update tail to point to prev
            }
        } else { // If no process was found before the one with the highest priority (i.e., it is at the head of the queue)
            proc = queue->head; // Set process to point to the head of the queue
            queue->head = queue->head->next; // Remove the head of the queue
            if (!queue->head) { // If there are no more processes in the queue
                queue->tail = NULL; // Set tail to NULL
            }
        }
        // proc->priority = max_priority;
    } else {
        // For other scheduling algorithms, remove and return the head of the queue
        proc = queue->head; // Set process to point to the head of the queue
        queue->head = queue->head->next; // Remove and return head of queue.
        if (!queue->head) {  // If there are no more processes in queue.
            queue->tail = NULL;  // Set tail to NULL.
        }

    }
    proc->priority = priority_with_age(cur_time, proc);
    proc->next = NULL;
    pthread_mutex_unlock(&queue_mutex);
    return proc;
}

/** ------------------------Problem 0-----------------------------------
 * Checkout PDF Section 2 for this problem
 * 
 * is_empty() is a helper function that returns whether the ready queue
 * has any processes in it.
 * 
 * @param queue pointer to the ready queue
 * 
 * @return a boolean value that indicates whether the queue is empty or not
 */
bool is_empty(queue_t *queue)
{
    return !queue->head;
}

/** ------------------------Problem 1B-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * schedule() is your CPU scheduler.
 * 
 * Remember to specify the timeslice if the scheduling algorithm is Round-Robin
 * 
 * @param cpu_id the target cpu we decide to put our process in
 */
static void schedule(unsigned int cpu_id)
{
    /* Dequeue a process from the ready queue. */
    pcb_t* proc = dequeue(rq);
    pthread_mutex_lock(&current_mutex);    
    /* If a process was dequeued, update its state to PROCESS_RUNNING. */
    if (proc) {
        proc->state = PROCESS_RUNNING;
        /* Update the current[] array to keep track of the process currently executing on the specified CPU. */
        current[cpu_id] = proc;
    }
    /* Unlock the current_mutex. */
    pthread_mutex_unlock(&current_mutex);
    /* Call context_switch() to switch to the selected process. */
    context_switch(cpu_id, proc, timeslice);

}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled. This function should block until a process is added
 * to your ready queue.
 *
 * @param cpu_id the cpu that is waiting for process to come in
 */
extern void idle(unsigned int cpu_id)
{
   // Lock the mutex to ensure mutual exclusion when accessing the ready queue
   pthread_mutex_lock(&queue_mutex);
   // Check if the ready queue is empty
   while (is_empty(rq)) {
    // If the ready queue is empty, wait for a signal that a process has been added to the queue
    pthread_cond_wait(&queue_not_empty, &queue_mutex);
   }
   // Unlock the mutex after accessing the ready queue
   pthread_mutex_unlock(&queue_mutex);
   // Call the schedule function to schedule a process on the CPU
   schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     *     // mt_safe_usleep(1000000);
     */
}

/** ------------------------Problem 2 & 3-----------------------------------
 * Checkout Section 4 and 5 for this problem
 * 
 * preempt() is the handler used in Round-robin and Preemptive Priority 
 * Scheduling
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 * 
 * @param cpu_id the cpu in which we want to preempt process
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    pcb_t* proc = current[cpu_id];
    proc->state = PROCESS_READY;
    enqueue(rq, proc);
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3 for this problem
 * 
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * @param cpu_id the cpu that is yielded by the process
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);

    pcb_t* proc = current[cpu_id];
    proc->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    /* Call schedule() to schedule a new process on the specified CPU. */
    schedule(cpu_id);

}

/**  ------------------------Problem 1A-----------------------------------
 * Checkout PDF Section 3
 * 
 * terminate() is the handler called by the simulator when a process completes.
 * 
 * @param cpu_id the cpu we want to terminate
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);    
    /* Get the currently executing process on the specified CPU. */
    pcb_t* proc = current[cpu_id];
    /* Update its state to PROCESS_TERMINATED. */
    proc->state = PROCESS_TERMINATED;
    proc = NULL;
    pthread_mutex_unlock(&current_mutex);
    /* Call schedule() to schedule a new process on the specified CPU. */
    schedule(cpu_id);
}

/**  ------------------------Problem 1A & 3---------------------------------
 * Checkout PDF Section 3 and 5 for this problem
 * 
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes. This method will also need to handle priority, 
 * Look in section 5 of the PDF for more info.
 * 
 * We have provided a function get_current_time() which is prototyped in os-sim.h.
 * Look there for more information.
 * 
 * @param process the process that finishes I/O and is ready to run on CPU
 */
extern void wake_up(pcb_t *process) {

    // for non PA scheduler algorithm
    process->state = PROCESS_READY;
    enqueue(rq, process);

    /* Check if the scheduler algorithm is priority scheduling. */
    if (scheduler_algorithm == PA) {
        pthread_mutex_lock(&current_mutex);

        for (unsigned int i = 0; i < cpu_count; i++) {
            if (current[i] == NULL) {
                pthread_mutex_unlock(&current_mutex);
                return;
            }
        }
        /* Initialize variables to keep track of the CPU running the process with the lowest priority. */
        unsigned int min_priority_cpu = (unsigned int) ~0, min_priority = (unsigned int) ~0;
        /* Lock the current_mutex to protect the current[] array from being accessed simultaneously by multiple threads. */
        /* Iterate over all CPUs and find the one running the process with the min priority. */
        for (unsigned int i = 0; i < cpu_count; i++) {
            double process_priority = priority_with_age(get_current_time(), current[i]);

            if (current[i] && process_priority < min_priority) {
                min_priority_cpu = i;
                min_priority = current[i]->priority;
            }
        }
        /* Unlock the current_mutex. */
        pthread_mutex_unlock(&current_mutex);

        /* Check if a CPU running a process with lower priority than the waking up process was found. */
        if (process->priority > min_priority) {
            /* If so, call force_preempt() to preempt that process. */
            force_preempt(min_priority_cpu);
        }
    }
}

/**
 * main() simply parses command line arguments, then calls start_simulator().
 * Add support for -r and -p parameters. If no argument has been supplied, 
 * you should default to FCFS.
 * 
 * HINT:
 * Use the scheduler_algorithm variable (see student.h) in your scheduler to 
 * keep track of the scheduling algorithm you're using.
 */
int main(int argc, char *argv[])
{
    /* FIX ME */
    scheduler_algorithm = FCFS;
    age_weight = 0;

    if (argc != 2 && argc != 4) {
        fprintf(stderr, "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
                        "Usage: ./os-sim <# CPUs> [ -r <time slice> | -p ]\n"
                        "    Default : FCFS Scheduler\n"
                        "         -r : Round-Robin Scheduler\n1\n"
                        "         -p : Priority Scheduler\n");
        return -1;
    }else if (argc == 4 && !strcmp(argv[2], "-r")) {
        scheduler_algorithm = RR;
        timeslice = atoi(argv[3]);
    }else if (argc == 4 && !strcmp(argv[2], "-p")) {
        scheduler_algorithm = PA;
        age_weight = (unsigned int) atoi(argv[3]);
    }

    /* Parse the command line arguments */
    cpu_count = strtoul(argv[1], NULL, 0);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t *) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);
    rq = (queue_t *)malloc(sizeof(queue_t));
    assert(rq != NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}

#pragma GCC diagnostic pop
