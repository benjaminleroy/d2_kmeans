#!/bin/sh

## Parameters 
num_of_nodes=8 # MPI processors
meta_class=500 # variability of population

dim_of_vec=8
size_of_bag=16
k=16
total_size=10000
max_iter=20

## Generate data
cd ../matlab
octave --silent --eval "syntheticdata($dim_of_vec, $size_of_bag, $meta_class, $total_size)"
echo 'Data generated'

cd .. && make clean && make MPI=1 &> /dev/null
echo 'Build done'

## Split data
mkdir -p data/synthetic_data
batch_size=`./d2 -i data/synthetic_data/${total_size}_${dim_of_vec}_${size_of_bag}_${meta_class}.d2 -p 1 -n ${total_size} -d ${dim_of_vec} -s ${size_of_bag} --prepare_batches $num_of_nodes | grep batch_size | sed 's/^.*batch_size://g'`
echo "Split data with batch size $batch_size over $num_of_nodes processors"

filename="${total_size}_${dim_of_vec}_${size_of_bag}_${meta_class}"
time mpirun -n $num_of_nodes ./d2 -i data/synthetic_data/${filename}.d2 -p 1 -n ${total_size} -d ${dim_of_vec} -s ${size_of_bag} --clusters $k --max_iter $max_iter -o data/synthetic_data/${filename}_p.d2

