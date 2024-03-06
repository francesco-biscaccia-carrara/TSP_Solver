#ifndef __UTILS_H 

#define __UTILS_H

#include "tsp.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>  
#include <float.h> 
#include <time.h>

void tsp_free_instance(instance * inst);

void print_error(const char *err);

uint64_t get_time();

void tsp_read_file(instance * inst);

void tsp_parse_cli(int argc, char** argv, instance *inst);

void tsp_plot(instance *inst);

#endif