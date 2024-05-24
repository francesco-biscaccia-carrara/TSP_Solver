import subprocess
import csv

time = 0;
cost = 1;




#EDITABLE PARAMETER
time_limit = 30;
node_size = 400;
wantCost = True;
seeds = [23, 24, 25];
algos = ["DIVING", "GREEDY"];

warm = "-warm" ##remove to remove warm
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
