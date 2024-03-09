#include "../include/tsp.h"

#define MAX_DIST 10000

void tsp_generate_random_point(uint32_t nnodes, uint32_t seed, instance* inst) {
    inst->points = malloc(nnodes * sizeof(point));
    inst->nnodes = nnodes;
    inst->points = (point *) calloc(inst->nnodes, sizeof(point));
    inst->edge_weights = (int *) calloc(((inst->nnodes*(inst->nnodes-1)/2)), sizeof(int));
    srand(seed);

    int i = 0; for(; i < nnodes; i++) {
        point p = {rand() % MAX_DIST, rand() % MAX_DIST};
        inst->points[i] = p;
        
        if(VERBOSE > 5)
            printf("(%i, %i)\n", p.x, p.y);
    }
}

/*
void generate_tsp_file(char *name) {
    FILE *fptr;
    char * filename = malloc(strlen(name) + 5); //.tsp\0
    strcpy(filename,name);
    strcat(filename,".tsp");

    fptr = fopen(filename, "w");

    fprintf(fptr, "Some text");
    fclose(fptr);
}
*/