#!/bin/bash
algorithms="pq-tsl_wcas,pq-linden,pq-lotan"
exp_date=$1 #experiment/data date passed from the terminal
key_range=1073741824

small_size=(12000)
large_size=(12000000)
low_insert_percent=(0 25 50)
high_insert_percent=(50 75 100)
thread_affinity=('dense')

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