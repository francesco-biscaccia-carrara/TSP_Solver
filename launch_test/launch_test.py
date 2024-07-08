import subprocess
import argparse
import numpy as np
import notify
import csv

bot = notify.bot(profile="default")

parser = argparse.ArgumentParser(description='TEST LAUNCHER')
parser.add_argument('nnodes', type=int, help='Number of nodes')
parser.add_argument('-tl',type=int,default=3.6e+6 ,help='Time limit for each execution')
parser.add_argument('-cost', dest='cost', action='store_true', help='Set the wantCost value to True.')
#parser.add_argument('algos',type=str,nargs='+',help='Algorithm to test, divided by space')
args = parser.parse_args()

#----------EDITABLE PARS--------#
node_size = args.nnodes
time_limit = args.tl
#algos = args.algos
warm = "-warm" ##remove to remove warm
wantCost = args.cost
#-------------------------------#


#----------FIXED PARS--------#
time = 0
cost = 1
seeds = np.arange(1,21)
methods=[15,10,5,2]
filename = '_'.join("TABU_B")+'-n_'+str(node_size)
#----------------------------#

results = []
row = 0
try:
    bot.create_progress_bar(steps=len(seeds))
    for seed in seeds:
        results.append([])
        results[row].append(seed)
        for par in methods:
            result = subprocess.run(
                ["../main", "-tl", str(time_limit), "-n", str(node_size), "-seed", str(seed), "-algo","TABU_B", warm, "-t","-tp",str(par)],
                capture_output = True,
                text = True 
            ) 
            values = result.stdout.split(",")

            if wantCost:
                results[row].append(float (values[cost]))
            else:
                results[row].append(float (values[time]))

        row += 1
        bot.update_progress_bar()
    bot.conclude_progress_bar()

    with open(filename+'.csv', 'w') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow([len(methods)] + methods)
        csvwriter.writerows(results)


    if wantCost: 
        x_lab = 'Cost Ratio'
    else: 
        x_lab = 'Time Ratio'

    subprocess.run(
            ["python3","perfprof.py","-D",",","-X "+x_lab,"-P ","-S 2",filename+".csv",filename+".png"],
            capture_output = False,
            text = True 
            ) 

    notify.bot(profile="gruppo_bmz").send_document_by_path(filename+".png", caption=filename, disable_notification=True)
except Exception as e:
    bot.send_exception(repr(e))