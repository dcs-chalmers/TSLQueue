#!/bin/bash

exp_date=$(date "+%Y-%m-%d_%H-%M-%S")
ex_repeats=3
printf '\nExperiment date = %s\n' "$exp_date"
printf '\nRepeats = %s\n\n' "$ex_repeats"

algorithms="pq-tsl_wcas,pq-linden,pq-lotan"

NUM_THREADS=$1 #threads list passed from the terminal
key_range=1073741824

small_size=(12000)
large_size=(12000000)
low_insert_percent=(0 25 50)
high_insert_percent=(50 75 100)
thread_affinity=('dense') #sparse

##run experiments for various tests and record data in formatted csv files
for exp_type in throughput; do
	printf '\n******************************************************'
	printf '\nExperiment = %s' "$exp_type"
	printf '\n******************************************************'
	for affinity in ${thread_affinity[@]}; do  
		printf '\n******************************************************'
		printf '\nMemory setup = %s' "$affinity"
		printf '\n******************************************************'
		printf '\n******************************************************\nCompiling tests \n******************************************************\n\n'
		cd ../benchmark && make lf_pq VERSION=O3 INIT=all VALIDATESIZE=0 MEMORY_SETUP=$affinity
		printf '\n******************************************************\nFinished compiling tests \n******************************************************\n'
		printf '\n******************************************************\nRunning tests \n******************************************************\n'
		for initial_size in ${large_size[@]}; do
			printf '\n******************************************************'
			printf '\n Initial queue size = %s elements \n' "$initial_size"
			for insert_percent in ${low_insert_percent[@]}; do
				printf '\t -Insert percentage = %s \n' "$insert_percent"
				cd ../scripts && python ./pq-experiments.py -a $algorithms -e $exp_type -n $NUM_THREADS -d $exp_date -m $affinity -l $ex_repeats -i $initial_size -r $key_range  -p $insert_percent
			done
		done
		for initial_size in ${small_size[@]}; do
			printf '\n******************************************************'
			printf '\n Initial queue size = %s elements \n' "$initial_size"
			for insert_percent in ${high_insert_percent[@]}; do
				printf '\t -Insert percentage = %s \n' "$insert_percent"
				cd ../scripts && python ./pq-experiments.py -a $algorithms -e $exp_type -n $NUM_THREADS -d $exp_date -m $affinity -l $ex_repeats -i $initial_size -r $key_range  -p $insert_percent
			done
		done
	done
done
printf '\n******************************************************\nFinished running tests at  %s\n******************************************************\n\n' "$(date "+%Y-%m-%d %H:%M:%S")"

##plot graphs from the generated csv data files
for exp_type in throughput; do
	printf '\n******************************************************'
	printf '\nPlotting graphs for experiment date = %s' "$exp_date"
	printf '\n******************************************************'
	for affinity in ${thread_affinity[@]}; do
		for initial_size in ${large_size[@]}; do
			for insert_percent in ${low_insert_percent[@]}; do
				cd ../scripts && python ./pq-graphs.py -a $algorithms -e $exp_type  -d $exp_date -m $affinity -i $initial_size -r $key_range  -p $insert_percent
			done
		done
		for initial_size in ${small_size[@]}; do
			for insert_percent in ${high_insert_percent[@]}; do
				cd ../scripts && python ./pq-graphs.py -a $algorithms -e $exp_type  -d $exp_date -m $affinity -i $initial_size -r $key_range  -p $insert_percent
			done
		done
	done
done
printf '\n******************************************************\nFinished plotting graphs at  %s\n******************************************************\n\n' "$(date "+%Y-%m-%d %H:%M:%S")"