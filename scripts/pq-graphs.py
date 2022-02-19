#!/usr/bin/env python2

import matplotlib
matplotlib.use('Agg') 
import matplotlib.pyplot as plt
import numpy as np
import csv
from optparse import OptionParser
from array import *
plt.rc('xtick', labelsize=8) 
plt.rc('ytick', labelsize=8)
plt.rcParams["figure.figsize"] = (3.5,2.5)
from matplotlib import ticker
formatter = ticker.ScalarFormatter(useMathText=True)
formatter.set_scientific(True)
formatter.set_powerlimits((0,0)) 

color_list=('r','g','b','k','c','y','m','brown','orange','darkblue')
marker_list=('s','^','d','o','+','x','p','.','|',',')
linestyle_list=('-','-','-','-','-','-','-','-','-','-')
linestyle_list2=(':',':',':',':',':',':',':',':',':',':')


PATH_HOME = ".."
DATA_PATH = PATH_HOME+'/data/'

ALL_ALGORITHMS = ['pq-tsl_wcas','pq-lotan','pq-linden']#'pq-tsl_cas',

THREADS = [1,2]
REPS = 1
AFFINITY = 'dense'
SIDEWORK = 0
INSERT_PERCENT = 0
EXP_DURATION=1000
EXPERIMENT='throughput'
INITIAL = 12000;
RANGE = 1073741824; #2^30
DATETYM = '0000-00-00_00-00-00'
	
def throughput_plot():
	f, graph_list = plt.subplots(1, 1, sharex='col', sharey='row')
	graph_list.grid(True)
	graph_list.set_title('Insert='+str(insert_percent)+'% Initial='+str(initial),fontsize=6)
	
	datareader_label = np.genfromtxt (DATA_PATH+'pq-'+str(experiment)+'-p'+str(insert_percent)+'-r'+str(range)+'-i'+str(initial)+'-'+str(affinity)+'-w'+str(sidework)+'-'+datestr+'.csv', delimiter=",", dtype="|S20")
	datareader = np.genfromtxt (DATA_PATH+'pq-'+str(experiment)+'-p'+str(insert_percent)+'-r'+str(range)+'-i'+str(initial)+'-'+str(affinity)+'-w'+str(sidework)+'-'+datestr+'.csv', delimiter=",")
	
	thread_array = datareader[0][1:] #skip the firt item in the list/array of the first row
	
	a=''
	y_max=0
	y_min=1000000
	for c, row in enumerate(datareader[1:]): #skip the first row 
		algo=datareader_label[c+1][0]
		algorithm_throughput_array = row[1:]
		if algo=='pq-tsl_wcas':
			a='TSLqueue'
		elif algo=='pq-tsl_cas':
			a='TSLqueueCas'
		elif algo=='pq-linden':
			a='Linden'
		elif algo=='pq-lotan':
			a='Lotan'
		
		graph_list.plot(thread_array, algorithm_throughput_array, color=color_list[c], linestyle=linestyle_list[c], marker=marker_list[c], markersize=4, label=str(a), linewidth=1.5, mec=color_list[c])
		
		y_min = min(np.amin(algorithm_throughput_array),y_min)
		y_max = max(np.amax(algorithm_throughput_array),y_max)
		
	plt.xticks(thread_array)
	graph_list.set_ylim([y_min,y_max])
	graph_list.set_xlim([np.amin(thread_array),np.amax(thread_array)])
	graph_list.set_ylabel('Throughput (Mop/s)',fontsize=8)
	graph_list.set_xlabel('Threads ',fontsize=8)
	#place legend key outside plot area on the upper right next to the first graph
	graph_list.legend(loc='lower center', ncol=4, borderaxespad=0.,fontsize=7,bbox_to_anchor=(0.5,-0.13,),bbox_transform = plt.gcf().transFigure)
	#save graph
	plt.savefig(PATH_HOME+'/graphs/pq-'+str(experiment)+'-p'+str(insert_percent)+'-r'+str(range)+'-i'+str(initial)+'-'+str(affinity)+'-w'+str(sidework)+'-'+datestr+'.png', bbox_inches='tight',dpi=300)
	plt.close() #close figure

#start of the main function
if __name__ == '__main__':
	parser = OptionParser()
	parser.add_option("-a", "--algorithms", dest = "algorithms", default = ",".join(ALL_ALGORITHMS), help = "Comma-separated list of %s" % ALL_ALGORITHMS)
	parser.add_option("-p", "--insertpercent", dest = "insert_percent", default = INSERT_PERCENT, help = "Insert percentage (rate)")
	parser.add_option("-r", "--range", dest = "range", type = 'int', default = RANGE, help = "key range")
	parser.add_option("-d", "--date", dest = "datetym", default = DATETYM, help = "Date and time of experiment")
	parser.add_option("-m", "--affinity", dest = "affinity", default = AFFINITY, help = "Experiment thread affinity")
	parser.add_option("-w", "--sidework", dest = "sidework", default = SIDEWORK, help = "Experiment sidework")
	parser.add_option("-e", "--experiment", dest = "experiment", default = EXPERIMENT, help = "Experiment being run")
	parser.add_option("-i", "--initial", dest = "initial", default = INITIAL, help = "Initial number of elements in the set")
	(options, args) = parser.parse_args()
	
	#validating algorithms and number of threads against the predefined
	datestr = options.datetym
	affinity=options.affinity
	sidework=options.sidework
	experiment=options.experiment
	initial=options.initial
	range=options.range
	insert_percent = options.insert_percent
	algorithms = options.algorithms.split(',')
	for a in algorithms:
		if a not in ALL_ALGORITHMS:
			parser.error('Invalid algorithm '+str(a))
	
	throughput_plot()