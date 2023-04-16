# 1. Parallel Implementations of Adaptive Quadrature using OpenMP  
The goal of this project is to implement two versions of a divide-and-conquer
algorithm in OpenMP.  

We are provided with two codes which implement the same algorithm in two different ways.    
The algorithm is an adaptive quadrature method that computes the integral of a function  
on a closed interval using a divide-and-conquer method. The algorithm starts by applying  
two quadrature rules (3-point and 5-point Simpson’s rules) to the whole interval. If the  
difference between the integral estimates from  the two rules is small enough (or the  
interval is too short), the result is added to the total integral estimate. If it is not  
small enough, the interval is split into two equal halves, and the method is applied  
recursively to each half. In the case supplied, evaluating the function requires the  
solution of an ODE (ordinary differential equation) which is relatively expensive in time.    

As supplied in the **serial** folder, the sequential code in **solver1.c** implements this  
algorithm using recursive function calls. The sequential code in **section2.c** implements   
the algorithm using a LIFO (last-in-first-out) queue.    

The **solver1.c**, **solver2.c**, and **solver3.c** files in the top folder are parallelised versions  
of the suplied sequential programs.  


# 2. Header and Source Files  
In this section, we describe the header and source files of the Adaptive Quadrature application.  

**function.h**  
This is the header file for the simulation, which contains the function  
prototypes for euler() and func1(), which are used for evaluating the  
function.    

**function.c**  
The file contains the code for evaluating the function. We treat this file as a “black box”  
and do not modify any of it.  

**solver1.c**  
This code parallelises the sequential code in the serial folder (also called **solver1.c**)  
using recursive function calls. We use OpenMP task constructs to parallelise this version.  

**solver2.c**  
This code parallelises the sequential code in the serial folder (also called **solver2.c**)  
using using a LIFO (last-in-first-out) queue.  

We parallelise this version without using task constructs. We enclose the do while loop  
in an OpenMP parallel region and allow all the threads to enqueue and dequeue intervals.  
We synchronise the accesses to the queue with OpenMP critical sections.  

Note that the termination condition for the while loop (an empty queue) is not sufficient  
in the parallel version, as the queue might be empty, but another thread may subsequently  
enqueue new intervals. The do while loop in the parallel version terminates only when the  
queue is empty and there are no intervals currently being processed.  

**solver3.c**  
This code parallelises the sequential code in the serial folder (called **solver2.c**) using  
a LIFO (last-in-first-out) queue.  

The difference between this version and the parallelised **solver2.c** version in the top folder  
is that instead of using OpenMP critical sections, we use OpenMP lock routines to synchronise the  
accesses to the queue.  


# 3. Compilation  
In this section, we discuss how to compile the Adaptive Quadrature code on the Cirrus HPC  
system.  

**3.1 Load relevant modules first:**  
If using GNU 10.2 compiler, type in command line:  
```module load gcc/10.2.0```  

If using Intel 20.4 compiler, type in command line:  
```module load intel-20.4/compilers```  

**3.2 To compile the code:**   
(1) Build the executables. Type in command line:  
```make```  

**3.3 To compile any specific solver version:**  
(1) Build the executable (e.g. Solver 1). Type in command line:  
```make solver1```  


# 4. Execution  
In this section, we discuss how to run the code on both the frontend (i.e login)  
and backend (i.e. compute) nodes of Cirrus.  

**4.1 To run the code on the login node:**    
(1) Set the number of threads (e.g. 4 threads) to use in the OMP_NUM_THREADS environment   
variable:  
```export OMP_NUM_THREADS=4```  

(2) Run the specific executable (e.g. Solver 1):  
```./solver1```  

**4.3 To run the code on the backend node:**  
(1) For running the specific executable (e.g. Solver1), type in command line:  
```sbatch solver1.slurm```  

By default, the slurm batch scripts are configured to load **gcc/10.2.0** compiler on the  
backend node of Cirrus. To use Intel 20.4 compiler, open the batch script in any supported  
code editor (e.g. Vim), and comment the ```module load gcc/10.2.0``` command, and remove the    
comment from the ```module load intel-20.4/compilers``` command.  

By default, the batch scripts are set to use a number of threads  
(i.e. 1, 2, 4, 6, 8, 12, 16, 24, 32), and runs each thread configuration 3 times in a  
for loop. You can change this setting as well with your desired number of threads, and the  
amount of times you want to run each configuration.  

**4.4 Additional information**  
For more information on running codes on the Cirrus HPC system, please visit  
this [page](https://cirrus.readthedocs.io/en/main/user-guide/batch.html).  
