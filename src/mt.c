#include "../include/mt.h"
#include "../include/utils.h"

void init_mt_context(mt_context* ctx,int num_threads){
    ctx->num_threads=num_threads;
    ctx->threads = (pthread_t*) malloc(num_threads*sizeof(pthread_t));
    pthread_mutex_init(&ctx->mutex, NULL);
}

void assign_task(mt_context* ctx,int th_i,void* (*funct)(void*) ,void* args){
    if(pthread_create(&ctx->threads[th_i],NULL,funct,args)) print_error("bad tasks assignment!");
}

void delete_mt_context(mt_context* ctx){
    for(int k=0;k<ctx->num_threads;k++) if(pthread_join(ctx->threads[k],NULL))  print_error("bad join of threads!");
    pthread_mutex_destroy(&ctx->mutex);
    free(ctx->threads);
}

extern uint16_t best_num_threads(size_t n){
    uint16_t real_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if(n<50) return 0;
    if(n>50 && n < 100) return sysconf(_SC_NPROCESSORS_ONLN);
    else return 2*sysconf(_SC_NPROCESSORS_ONLN);
}