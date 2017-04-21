#!/bin/bash
#SBATCH -N 2
#SBATCH -p RM
#SBATCH --ntasks-per-node=1
#SBATCH -t 04:00:00
#SBATCH --mail-type=FAIL
#SBATCH --mail-user=kgills@gmail.com

ITERS=$(seq 1 10)
SIZE=2048
FILE_NAME="quick_mpi"
NODES=8

export OMP_NUM_THREADS=28
touch "${FILE_NAME}_${NODES}_${SIZE}.txt"
echo "matrix_dim, etime, flops, cores">>"${FILE_NAME}_${NODES}_${SIZE}.txt"
for ITER in ${ITERS}
do
    mpirun -np ${NODES} --mca mpi_cuda_support 0 ./quick.out>>"${FILE_NAME}_${NODES}_${SIZE}.txt"
done
