#ifndef __MTHRD_H 

#define __MTHRD_H

#include <unistd.h>
#include <pthread.h>

#include "tsp.h"


typedef struct{
    int num_threads;
    pthread_mutex_t mutex;
    pthread_t* threads;
} mt_context;

extern void init_mt_context(mt_context* ctx,int num_threads);
extern void assign_task(mt_context* ctx,int th_i,void* (*funct)(void*) ,void* args);
extern void delete_mt_context(mt_context* ctx);
extern uint16_t best_num_threads(size_t n);

#endif