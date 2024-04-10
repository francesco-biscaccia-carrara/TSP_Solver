import matplotlib.pyplot as plt
import numpy as np
import re

xval = []
yval = []


f = open("../test.txt", "r")
lines = f.readlines()

for line in lines:
    v = re.findall("(.*),(.*);(.*),(.*)", line)
    print(v)
    xval.append(float(v[0][0]))
    xval.append(float(v[0][2]))
    yval.append(float(v[0][1]))
    yval.append(float(v[0][3]))

plt.plot(xval, yval, "-o")
plt.show()
    
