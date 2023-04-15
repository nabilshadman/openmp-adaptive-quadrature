#!/bin/bash

# Slurm job options (name, compute nodes, job time)
#SBATCH --job-name=solver1
#SBATCH --time=0:20:0
#SBATCH --exclusive
#SBATCH --nodes=1
#SBATCH --tasks-per-node=1
#SBATCH --cpus-per-task=36

# Replace [budget code] below with your budget code (e.g. dc116-s1234567)
#SBATCH --account=m22ol-s2134758
# We use the "standard" partition as we are running on CPU nodes
#SBATCH --partition=standard
# We use the "short" QoS as our runtime is less than20 mins 
#SBATCH --qos=standard

# Load appropriate module
module load intel-20.4/compilers
#module load gcc/10.2.0

# Change to the submission directory
cd $SLURM_SUBMIT_DIR

# Set the number of threads
# Launch the parallel job
for threads in 1 2 4 6 8 12 16 24 32
do 
	export OMP_NUM_THREADS=$threads 
	srun --cpu-bind=cores ./solver1
	srun --cpu-bind=cores ./solver1
	srun --cpu-bind=cores ./solver1
done

