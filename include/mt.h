/** @file mt.h
 *  @brief Multithreading template.
 *
 *  This is a template that allows you to easily write a multithreading function in your C program.
 *
 *  @author Francesco Biscaccia Carrara
 *  @bug No known bugs.
 */

#ifndef __MT_H 

#define __MT_H

#include <unistd.h>
#include <pthread.h>

#include "tsp.h"


typedef struct{
    int num_threads;
    pthread_mutex_t mutex;
    pthread_t* threads;
} mt_context;

/// @brief Initialize the multitrheading context (which is a set of num_threads threads and a mutex)
/// @param ctx Pointer to mt_context instance
/// @param num_threads Numbero of threads you want to use
extern void init_mt_context(mt_context* ctx,int num_threads);


/// @brief Assign a task (the function funct) to the thread th_i and run it.
/// @param ctx Pointer to mt_context instance
/// @param th_i Index of the thread
/// @param funct Function to run (MUST BE A void* funct(void*))
/// @param args void* pointer to data that can be passed to the function funct 
extern void assign_task(mt_context* ctx,int th_i,void* (*funct)(void*) ,void* args);


/// @brief Delete the multitrheading context (free the memory for the threads and destroy the mutex)
/// @param ctx Pointer to mt_context instance
extern void delete_mt_context(mt_context* ctx);


/// @brief Gives the number of threads that can be appropriate for n element 
/// @param n Number of items to be processed
/// @return 0   No multithreading needed
///         t   Number of suggested threads 
extern uint16_t sugg_num_threads(size_t n);

#endif