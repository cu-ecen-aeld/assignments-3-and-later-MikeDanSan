#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#ifdef DEBUG_LOG
#undef DEBUG_LOG
#endif
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)

#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    /* TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param; */


    // Cast the thread_param to thread_data structure
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    // Wait for the specified time before attempting to lock the mutex
    usleep(thread_func_args->wait_to_obtain_ms * 1000); // Convert ms to microseconds

    // Attempt to lock the mutex
    if (pthread_mutex_lock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to lock mutex");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Wait while holding the mutex
    usleep(thread_func_args->wait_to_release_ms * 1000); // Convert ms to microseconds

    // Unlock the mutex
    if (pthread_mutex_unlock(thread_func_args->mutex) != 0) {
        ERROR_LOG("Failed to unlock mutex");
        thread_func_args->thread_complete_success = false;
        return thread_param;
    }

    // Mark the thread as successfully completed
    thread_func_args->thread_complete_success = true;

    // Return the thread_data structure pointer
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    DEBUG_LOG("Starting thread creation with wait_to_obtain_ms=%d, wait_to_release_ms=%d", wait_to_obtain_ms, wait_to_release_ms);

    // Allocate memory for the thread_data structure
    struct thread_data *thread_func_args = malloc(sizeof(struct thread_data));
    if (thread_func_args == NULL) {
        ERROR_LOG("Failed to allocate memory for thread_data");
        return false;
    }
    DEBUG_LOG("Allocated memory for thread_data structure");

    // Initialize the thread_data structure
    thread_func_args->mutex = mutex;
    thread_func_args->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_func_args->wait_to_release_ms = wait_to_release_ms;
    thread_func_args->thread_complete_success = false;

    DEBUG_LOG("Initialized thread_data structure with mutex=%p, wait_to_obtain_ms=%d, wait_to_release_ms=%d",
            (void *)mutex, wait_to_obtain_ms, wait_to_release_ms);

    // Create the thread using pthread_create
    if (pthread_create(thread, NULL, threadfunc, thread_func_args) != 0) {
        ERROR_LOG("Failed to create thread");
        free(thread_func_args); // Free allocated memory on failure
        return false;
    }

    DEBUG_LOG("Thread created successfully with thread ID=%lu", (unsigned long)*thread);

    // Return true to indicate success
    return true;
}

