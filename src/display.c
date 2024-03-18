#include "../include/display.h"
#include "../include/tsp.h"
#include "../include/utils.h"


#pragma region static_functions

void tsp_create_plot_data(const instance *problem){
    FILE* file = fopen(".tmp.dat","w");

    for(int i=0;i < problem->nnodes; i++){ 
        fprintf(file,"%10.4f %10.4f\n",problem->points[problem->solution[i]].x,problem->points[problem->solution[i]].y);
        if(i!=problem->nnodes-1) fprintf(file,"%10.4f %10.4f\n\n",problem->points[problem->solution[i+1]].x,problem->points[problem->solution[i+1]].y);
        else fprintf(file,"%10.4f %10.4f\n",problem->points[problem->solution[0]].x,problem->points[problem->solution[0]].y);
    }

    fclose(file);
}

#pragma endregion

void tsp_plot(const instance *problem, const cli_info* cli){
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

void print_best_solution_info(const instance* problem,const cli_info* cli){
    printf("\n\e[1mBest Solution Found\e[m (by \e[1m%s\e[m)\n",cli->method);
    printf("Starting node:\t%i\n",problem->solution[0]);
	printf("Cost: \t%10.4f\n", problem->cost);
}

