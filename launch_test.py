import subprocess
import argparse
import csv

parser = argparse.ArgumentParser(description='TEST LAUNCHER')
parser.add_argument("nnodes", type=int, help="Number of nodes")
parser.add_argument("tl",type=int, help="Time limit for each execution")
parser.add_argument("algos",type=str,nargs="+",help="Algorithm to test, divided by space")
args = parser.parse_args()

#EDITABLE PARAMETER
node_size = args.nnodes
time_limit = args.tl
algos = args.algos
warm = "-warm" ##remove to remove warm

#FIXED PARS
time = 0
cost = 1
wantCost = True
seeds = [1,2,3,4,5,6,7,8,9,10]
filename = 'out.csv'

results = []
row = 0
for seed in seeds:
    results.append([])
    results[row].append(seed)
    for algo in algos:
        result = subprocess.run(
            ["./main", "-tl", str(time_limit), "-n", str(node_size), "-seed", str(seed), "-algo", algo, warm, "-t"],
            capture_output = True,
            text = True 
        ) 
        values = result.stdout.split(",")

        if wantCost:
            results[row].append(values[cost])
        else:
            results[row].append(values[time])

    row += 1


print(results)

with open(filename, 'w') as csvfile:
    csvwriter = csv.writer(csvfile)
    csvwriter.writerow([len(algos)] + algos)
    csvwriter.writerows(results)

#TODO:
#   1. list of strategies,
#   2. type of perfprof,
#   3. save into csv
#   4. lauch perfprof.py
