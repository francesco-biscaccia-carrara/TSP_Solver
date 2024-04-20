import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import re


f = open("input/test.txt", "r")
lines = f.readlines()

for line in lines:
    v = re.findall("(.*),(.*);(.*),(.*)", line)
    xval = []
    yval = []
    xval.append(float(v[0][0]))
    xval.append(float(v[0][2]))
    yval.append(float(v[0][1]))
    yval.append(float(v[0][3]))
    plt.plot(xval, yval, "-o", color = "xkcd:teal")

plt.title("greedy solution")
#plt.savefig("out/test.pgf", backend='pgf')
#plt.savefig("out/test.png") Could be useful
plt.show();   
