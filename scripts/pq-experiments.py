#!/usr/bin/env python2

import datetime
import time
import re
import subprocess
import shlex
import getpass

from optparse import OptionParser

PATH_HOME = ".."
BIN = PATH_HOME+'/benchmark/bin/'
DATA_PATH = PATH_HOME+'/data/'

ALL_ALGORITHMS = ['pq-tsl_wcas','pq-tsl_cas','pq-lotan','pq-linden']#'pq-tsl_cas',

THREADS = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36]
REPS = 1
AFFINITY = 'dense'
SIDEWORK = 0
INSERT_PERCENT = [0,25,50,75,100]
EXP_DURATION=1000
EXPERIMENT='throughput'
INITIAL = 12000;
RANGE = 1073741824; #2^30
DATETYM = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')



#function that runs the individual algorithms
def bench(algorithm, put_rate, nthreads):
	output = subprocess.check_output([BIN+algorithm ,'-d', str(duration), '-i', str(initial), '-n', str(nthreads), '-p', str(put_rate)])
	return output.strip()


#start of the main function
if __name__ == '__main__':
	parser = OptionParser()
	parser.add_option("-a", "--algorithms", dest = "algorithms", default = ",".join(ALL_ALGORITHMS),
			help = "Comma-separated list of algorithms %s" % ALL_ALGORITHMS)
	parser.add_option("-n", "--nthreads", dest = "nthreads", default = ",".join(map(str, THREADS)),
            help = "Comma-separated list of number of threads")
	parser.add_option("-p", "--insertpercent", dest = "insert_percent", default = ",".join(map(str, INSERT_PERCENT)),
            help = "Comma-separated insert percentage (rate)")
	parser.add_option("-l", "--reps", dest = "reps", type = 'int', default = REPS,
            help = "Repetitions per run")
	parser.add_option("-r", "--range", dest = "range", type = 'int', default = RANGE,
            help = "key range")
	parser.add_option("-d", "--date", dest = "datetym", default = DATETYM,
            help = "Date and time of experiment")
	parser.add_option("-m", "--affinity", dest = "affinity", default = AFFINITY,
            help = "Experiment thread affinity")
	parser.add_option("-w", "--sidework", dest = "sidework", default = SIDEWORK,
            help = "Experiment sidework")
	parser.add_option("-t", "--duration", dest = "duration", default = EXP_DURATION,
            help = "Experiment running duration")
	parser.add_option("-e", "--experiment", dest = "experiment", default = EXPERIMENT,
            help = "Experiment being run e.g throughput")
	parser.add_option("-i", "--initial", dest = "initial", default = INITIAL,
            help = "Initial number of elements in the queue (set)")
	(options, args) = parser.parse_args()
	
	#validating algorithms and number of threads against the predefined
	datestr = options.datetym
	nreps=options.reps
	affinity=options.affinity
	sidework=options.sidework
	duration=options.duration
	experiment=options.experiment
	initial=options.initial
	range=options.range
	insert_percent = options.insert_percent.split(',')
	nthreads = options.nthreads.split(',')
	algorithms = options.algorithms.split(',')
	for a in algorithms:
		if a not in ALL_ALGORITHMS:
			parser.error('Invalid algorithm '+str(a))
	
	#loop through algorithm executions passing arguments as required
	for p in insert_percent:
		outputfile=DATA_PATH+'pq-'+str(experiment)+'-p'+str(p)+'-r'+str(range)+'-i'+str(initial)+'-'+str(affinity)+'-w'+str(sidework)+'-'+datestr+'.csv'
		with open(outputfile, 'w') as f:
			f.write('Threads')
			for n in nthreads:
				f.write(','+str(n))
			f.write('\n')
			for a in algorithms:
				if a in algorithms:
					f.write(str(a))
					for n in nthreads:					
						mops = 0.0
						for r in xrange(nreps):
							outstr = '%s' % (bench(a,p,n))
							outlist = outstr.split('\n')
							mopslist = outlist[19].split(',')
							mops += float(mopslist[1])
						mops = mops/nreps
						f.write(','+str(mops))
					f.write('\n')
		f.close()
					
					
						
							
								
					