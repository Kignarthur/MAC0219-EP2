#! /bin/bash

set -o xtrace

IMAGE_SIZE=4096
MEASUREMENTS=15

NAMES=('mandel_ompi_seq' 'mandel_ompi_pth' 'mandel_ompi_omp')
RESULTS='../data'

LOG='triple_spiral.log'
X_MIN=-0.188
X_MAX=-0.012
Y_MIN=0.554
Y_MAX=0.754

module load mpi/openmpi-x86_64 && make

[ ! -d $RESULTS ] && mkdir $RESULTS

for NAME in ${NAMES[@]}; do
    SUB_RESULTS=$RESULTS/$NAME
    [ ! -d $SUB_RESULTS ] && mkdir $SUB_RESULTS

    [[ $NAME == ${NAMES[0]} ]] && MAX_THREADS=1 || MAX_THREADS=32

    for PROCS in 1 8 16 32 64; do
        for ((THREADS=1; THREADS<=$MAX_THREADS; THREADS*=2)); do
            perf stat -r $MEASUREMENTS mpirun --oversubscribe -np $PROCS ./$NAME $X_MIN $X_MAX $Y_MIN $Y_MAX $IMAGE_SIZE $THREADS >> $LOG 2>&1
        done
    done

    mv *.log $RESULTS/$NAME
done

python3 process_log.py
