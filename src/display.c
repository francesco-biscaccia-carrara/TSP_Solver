#include "../include/display.h"
#include "../include/tsp.h"
#include "../include/utils.h"

void tsp_create_plot_data(instance *problem){
    FILE* file = fopen(".tmp.dat","w");
    //TODO: different format of solution
    int j=0;
    for(int i=0;i < problem->nnodes; i++){ 
        fprintf(file,"%10.4f %10.4f\n",problem->points[j].x,problem->points[j].y);
        fprintf(file,"%10.4f %10.4f\n\n",problem->points[problem->combination[j]].x,problem->points[problem->combination[j]].y);
        j=problem->combination[j];
    }
    fclose(file);
}

void tsp_plot(instance *problem,cli_info* cli){
    char file_name[100];
    tsp_create_plot_data(problem);
    if(problem->random_seed) sprintf(file_name,"plot/n_%ld_s_%u_%s_plot.png",problem->nnodes,problem->random_seed,cli->method);
    else sprintf(file_name,"plot/%s_%s_plot.png",cli->file_name,cli->method);
    FILE* gnuplot_pipe = popen("gnuplot -persistent","w");
    fprintf(gnuplot_pipe,"set output '%s';",file_name);
    const char *COMMANDS[] ={
        "set term png;"
        "set title 'TSP Best Solution';",
        "set xrange [0:10000];",
        "set yrange [0:10000];",
        "unset key;",
        "plot '.tmp.dat' with linespoints linetype 7 linecolor 6;",0};
    for (int i=0;COMMANDS[i];i++) fprintf(gnuplot_pipe,"%s\n",COMMANDS[i]);
    pclose(gnuplot_pipe);
    system("rm .tmp.dat");
}

void print_best_solution_info(instance* problem){
    //TODO: different format of solution
    printf("\n\e[1mBest Solution Found\e[m\n");
    printf("starting node:\t%i\n",problem->combination[0]);
	printf("cost: \t%10.4f\n", problem->result);
}
