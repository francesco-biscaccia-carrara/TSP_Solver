# 🌍 The Traveling Salesman Problem: An Overview of Exact and Heuristic Approaches
![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)![Python](https://img.shields.io/badge/Python-3776AB?style=for-the-badge&logo=python&logoColor=white)

## 📚 Overview

This project explores various approaches to solve the Traveling Salesman Problem (TSP), one of the most famous NP-Hard problems in computer science and operations research. We implement and compare exact methods, heuristics, metaheuristics, and matheuristics to find optimal or near-optimal solutions for TSP instances.

### 🎓 Authors
- Francesco Biscaccia Carrara 
- Riccardo Modolo

*Department of Information Engineering, University of Padua*
*Academic Year: 2023/2024*

## 🧠 Problem Definition

The TSP is defined on a graph $G = (V,E)$ with a cost function $c:\;E \to \mathbb{R}^{+}$. The goal is to find a Hamiltonian cycle of minimum cost.

## 🚀 Implemented Approaches

### Exact Methods
1. **Bender's Loop**: Iteratively adds subtour elimination constraints.
2. **Branch and Cut**: Utilizes IBM ILOG CPLEX with custom callbacks.

### Heuristics
1. **Nearest Neighbors**: A greedy approach for quick solutions.
2. **2-OPT**: Improves existing tours by swapping edges.

### Metaheuristics
1. **Tabu Search**: Explores the solution space while avoiding cycles.
2. **Variable Neighborhood Search (VNS)**: Uses the concept of systematic change of neighborhood.

### Matheuristics
1. **Diving**: Fixes some variables to guide the MIP solver.
2. **Local Branching**: Provides flexibility in the number of variables to change.

## 🛠️ Implementation

The project is primarily implemented in C for performance, with Python scripts for analysis and visualization.

## 📊 Key Findings

1. **Exact Methods**: Optimal for instances with ≤300 nodes
2. **Matheuristics**: Best trade-off for 1000-4000 nodes
3. **Heuristics**: Recommended for large instances (10K-50K nodes)

Our performance profiles and comparisons show:
- VNS often outperforms Tabu Search
- Local Branching is competitive with Branch and Cut for shorter time limits
- Weighted Diving generally performs better than Random Diving

## 🔧 Usage

### Compilation

Use the provided Makefile:

```bash
make
```

This generates the `main` executable.

### Running the Solver

```bash
./main -n <nodes> -s <seed> -t <time_limit> -a <algorithm>
```

where:
- `<nodes>`: The number of nodes in the graph.
- `<seed>`: The random seed used to generate a random TSP instance.
- `<time_limit>`: The maximum execution time in seconds.
- `<algorithm>`: The algorithm to be used, such as nn, 2opt, tabu, vns, diving, or localbranching.

### Analysis

Use the Python scripts for visualization and analysis:

```bash
python3 plot/plot_solution.py input/<input_file>
python3 launch_test/perfprof.py <input_path> <output_path>.<extension>
```

## 📦 Dependencies

- C compiler
- Python 3.x
- IBM ILOG CPLEX Optimization Studio (for exact methods)
- Python libraries: matplotlib, numpy

## 🔮 Future Work

1. Enhance VNS performance by utilizing the history of applied kicks
2. Introduce "memory" to the Local Branching algorithm
3. Apply Machine Learning to optimize the application of the g2opt routine
4. Explore the effectiveness of these algorithms on various TSP variants

## 📜 Project Status

This project is part of an academic course at the University of Padua. It is not licensed for commercial use but serves as a comprehensive study of TSP solving techniques.

---

💡 For more details, please refer to the full report or contact the authors.
