# 🌍 The Traveling Salesman Problem Solver

## 📚 Overview

This project explores various approaches to solve the Traveling Salesman Problem (TSP), one of the most famous NP-Hard problems in computer science and operations research. We implement and compare exact methods, heuristics, metaheuristics, and matheuristics to find optimal or near-optimal solutions for TSP instances.

### 🎓 Authors
- Francesco Biscaccia Carrara
- Riccardo Modolo

*Department of Information Engineering, University of Padua, Academic Year 2023/2024*

## 🚀 Features

The project implements several algorithms to solve the TSP:

- **Exact Methods**: 
  - Bender's Loop
  - Branch and Cut (using IBM ILOG CPLEX)
- **Heuristics**:
  - Nearest Neighbors
  - 2-OPT
- **Metaheuristics**:
  - Tabu Search
  - Variable Neighborhood Search (VNS)
- **Matheuristics**:
  - Diving
  - Local Branching

## 🛠️ Implementation

The project is primarily implemented in C, with Python scripts for analysis and visualization.

### Project Structure

```
OR2_Project/
│
├── src/
│   ├── heuristic.c
│   ├── heuristic.h
│   ├── main.c
│   ├── metaheuristic.c
│   ├── metaheuristic.h
│   ├── opt_tsp.c
│   ├── opt_tsp.h
│   ├── patching.c
│   ├── patching.h
│   ├── utils.c
│   └── utils.h
│
├── scripts/
│   ├── performance_profile.py
│   └── plot_solution.py
│
├── instances/
│   └── (various .tsp files)
│
├── output/
│   └── (output files)
│
├── Makefile
└── README.md
```

## 📊 Results

The project includes scripts for visualizing results:

- `plot_solution.py`: Plots the TSP solution
- `performance_profile.py`: Generates performance profiles for algorithm comparison

## 🔧 Usage

### Compilation

Use the provided Makefile to compile the project:

```bash
make
```

This will generate the executable `tsp`.

### Running the Solver

To run the TSP solver:

```bash
./tsp -f <instance_file> -t <time_limit> -a <algorithm>
```

Where:
- `<instance_file>` is the path to a .tsp file in the instances directory
- `<time_limit>` is the maximum execution time in seconds
- `<algorithm>` is one of the implemented algorithms (e.g., nn, 2opt, tabu, vns, etc.)

### Analyzing Results

Use the Python scripts in the `scripts/` directory to analyze and visualize results:

```bash
python scripts/plot_solution.py <output_file>
python scripts/performance_profile.py <results_directory>
```

## 📦 Dependencies

- C compiler (gcc recommended)
- Python 3.x
- IBM ILOG CPLEX Optimization Studio (for exact methods)
- Python libraries: matplotlib, numpy (for visualization and analysis)

## 🏗️ Future Work

- Enhance VNS performance
- Optimize Local Branching implementation
- Implement additional TSP variants

## 📜 License

This project is part of an academic course and is not licensed for commercial use.

---

💡 For more details, please refer to the source code documentation or contact the authors.

