# 1. Parallel Implementations of Adaptive Quadrature using OpenMP
The goal of this [study](https://github.com/nabilshadman/openmp-adaptive-quadrature/blob/main/report/parallel_adaptive_quadrature_openmp_report.pdf) is to implement two versions of a divide-and-conquer algorithm in [OpenMP](https://www.openmp.org/).

**Tech Stack:** C, OpenMP, Make, Slurm, GitLab.

We are provided with two codes (in the C programming language) that implement the same algorithm in two different ways. The algorithm is an [adaptive quadrature](https://en.wikipedia.org/wiki/Adaptive_quadrature) method that computes the integral of a function on a closed interval using a divide-and-conquer method. The algorithm starts by applying two quadrature rules (3-point and 5-point Simpson's rules) to the whole interval. If the difference between the integral estimates from the two rules is small enough (or the interval is too short), the result is added to the total integral estimate. If it is not small enough, the interval is split into two equal halves, and the method is applied recursively to each half. In the case supplied, evaluating the function requires the solution of an ODE (ordinary differential equation), which is relatively expensive in time.

As supplied in the [serial](https://github.com/nabilshadman/openmp-adaptive-quadrature/tree/main/serial) folder, the sequential code in [solver1.c](https://github.com/nabilshadman/openmp-adaptive-quadrature/blob/main/serial/solver1.c) implements this algorithm using recursive function calls. The sequential code in [solver2.c](https://github.com/nabilshadman/openmp-adaptive-quadrature/blob/main/serial/solver2.c) implements the algorithm using a LIFO (last-in-first-out) queue.

The **solver1.c**, **solver2.c**, and **solver3.c** files in the top folder are parallelized versions of the supplied sequential programs.

## 2. Header and Source Files
In this section, we describe the header and source files of the Adaptive Quadrature application.

**function.h**  
This is the header file for the simulation, which contains the function prototypes for euler() and func1(), which are used for evaluating the function.

**function.c**  
The file contains the code for evaluating the function. We treat this file as a "black box" and do not modify any of it.

**solver1.c**  
This code parallelizes the sequential code in the serial folder (also called **solver1.c**), which uses recursive function calls. We use OpenMP task constructs to parallelize this version.

**solver2.c**  
This code parallelizes the sequential code in the serial folder (also called **solver2.c**), which uses a LIFO (last-in-first-out) queue.

We parallelize this version without using task constructs. We enclose the do-while loop in an OpenMP parallel region and allow all threads to enqueue and dequeue intervals. We synchronize accesses to the queue with OpenMP critical sections.

Note that the termination condition for the while loop (an empty queue) is not sufficient in the parallel version, as the queue might be empty, but another thread may subsequently enqueue new intervals. The do-while loop in the parallel version terminates only when the queue is empty and there are no intervals currently being processed.

**solver3.c**  
This code parallelizes the sequential code in the serial folder (called **solver2.c**), which uses a LIFO (last-in-first-out) queue.

The difference between this version and the parallelized **solver2.c** version in the top folder is that instead of using OpenMP critical sections, we use OpenMP lock routines to synchronize accesses to the queue.

## 3. Compilation
In this section, we discuss how to compile the Adaptive Quadrature application on the [Cirrus](https://www.epcc.ed.ac.uk/hpc-services/cirrus) HPC system.

**3.1 Load relevant modules first:**  
If using **GNU 10.2 compiler**, type in command line:
```
module load gcc/10.2.0
```

If using **Intel 20.4 compiler**, type in command line:
```
module load intel-20.4/compilers
```

**3.2 To compile the code:**  
(1) Build all the executables. Type in command line:
```
make
```

To compile any specific solver version instead (e.g., Solver 1), type in command line:
```
make solver1
```

By default, the **Makefile** is configured to compile the code using the **GNU compiler**. To compile using the **Intel compiler** instead, open the Makefile in any supported code editor (e.g., Vim), and comment the GNU compiler options and remove the comments from the Intel compiler options.

## 4. Execution
In this section, we discuss how to run the code on both the frontend (i.e., login) and backend (i.e., compute) nodes of [Cirrus](https://www.epcc.ed.ac.uk/hpc-services/cirrus).

**4.1 To run the code on the login node:**  
(1) Set the number of threads (e.g., 4 threads) you want to use in the **OMP_NUM_THREADS** environment variable:
```
export OMP_NUM_THREADS=4
```

(2) Run the specific executable (e.g., Solver 1):
```
./solver1
```

**4.2 To run the code on the backend node:**  
(1) For running the specific executable (e.g., Solver1), type in command line:
```
sbatch solver1.slurm
```

This command will send the solver executable to the **Slurm** job scheduler, where the code will be run as soon as resources are available. The command will return a <job_id>. When the code is run, the output will be printed to a file titled **slurm-<job_id>.out**.

By default, the slurm batch scripts are configured to load **GNU 10.2 compiler** on the backend node of Cirrus. To use **Intel 20.4 compiler**, open the batch script in any supported code editor (e.g., Vim), and comment the `module load gcc/10.2.0` command, and remove the comment from the `module load intel-20.4/compilers` command.

By default, the batch scripts are set to use a number of **threads** (i.e., 1, 2, 4, 6, 8, 12, 16, 24, 32), and run each thread configuration 3 times in a for loop. You can change this setting with your desired number of threads and the number of times you want to run each configuration.

**4.3 Additional information**  
We have included the results (i.e., slurm output files) from our experiments in the folder called **miscellaneous**. Also, the folder includes an Excel file (called **coursework2_data.xlsx**) that includes the timing data of the experiments and speedup graphs. The timing data is mapped to the slurm output file ID to reference for any future studies.

For more information on running codes on the Cirrus HPC system, please visit this [page](https://cirrus.readthedocs.io/en/main/user-guide/batch.html).

## 5. Report
We provide a [report](https://github.com/nabilshadman/openmp-adaptive-quadrature/blob/main/report/parallel_adaptive_quadrature_openmp_report.pdf) associated with this repository where we discuss the general algorithm, the parallel implementations of the algorithm, the hardware and software environments where we test the implementations for correctness and performance, the performance analysis of the implementations, and the conclusions of the study.
