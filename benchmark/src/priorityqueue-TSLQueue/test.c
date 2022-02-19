/*   
 *   File: test.c
 *   Author: Adones Rukundo 
 *	 <adones@chalmers.se, adones@must.ac.ug, adon.rkd@gmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h> 
#include <stdlib.h> 
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <sys/time.h>
#include <unistd.h>
#include <malloc.h>
#include "utils.h"

#include "priorityqueue-tslqueue.h"

#if !defined(VALIDATESIZE)
	#define VALIDATESIZE 1
#endif

/* ################################################################### *
 * GLOBALS
 * ################################################################### */

size_t initial = DEFAULT_INITIAL;
size_t range = DEFAULT_RANGE; 
size_t update = DEFAULT_UPDATE;
size_t num_threads = DEFAULT_NB_THREADS; 
size_t duration = DEFAULT_DURATION;

size_t put, put_explicit = false;
double update_rate, put_rate, get_rate;

size_t size_after = 0;
int seed = 0;
uint32_t rand_max;
#define rand_min 1
static volatile int stop;

volatile ticks *putting_count;
volatile ticks *putting_count_succ;
volatile ticks *getting_count;
volatile ticks *getting_count_succ;
volatile ticks *removing_count;
volatile ticks *removing_count_succ;
volatile ticks *total;


/* ################################################################### *
 * LOCALS
 * ################################################################### */
__thread unsigned long *seeds;
__thread ssmem_allocator_t* alloc;

barrier_t barrier, barrier_global;

typedef struct thread_data
{
	uint32_t id;
	DS_TYPE* set;
} thread_data_t;

void* test(void* thread) 
{
	thread_data_t* td = (thread_data_t*) thread;
	uint32_t thread_id = td->id;
	set_cpu(thread_id);
	ssalloc_init();
	DS_LOCAL();
	
	DS_TYPE *set = td->set;

	uint64_t my_putting_count = 0;
	uint64_t my_getting_count = 0;
	uint64_t my_removing_count = 0;

	uint64_t my_putting_count_succ = 0;
	uint64_t my_getting_count_succ = 0;
	uint64_t my_removing_count_succ = 0;
    
	seeds = seed_rand();
	#if GC == 1
		alloc = (ssmem_allocator_t*) malloc(sizeof(ssmem_allocator_t));
		assert(alloc != NULL);
		ssmem_alloc_init_fs_size(alloc, SSMEM_DEFAULT_MEM_SIZE, SSMEM_GC_FREE_SET_SIZE, thread_id);
	#endif   

	barrier_cross(&barrier);

	DS_KEY key;
	int c = 0;
	uint32_t scale_rem = (uint32_t) (update_rate * UINT_MAX);
	uint32_t scale_put = (uint32_t) (put_rate * UINT_MAX);

	int i;
	uint32_t num_elems_thread = (uint32_t) (initial / num_threads);
	int32_t missing = (uint32_t) initial - (num_elems_thread * num_threads);
	if (thread_id < missing)
    {
		num_elems_thread++;
    }

	#if INITIALIZE_FROM_ONE == 1
		num_elems_thread = (thread_id == 0) * initial;
	#endif    

	for(i = 0; i < num_elems_thread; i++) 
    {
		key = (my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) % (rand_max + 1)) + rand_min;     
		if(DS_ADD(set, key, NULL) == false)
		{
			i--;
		}
    }
	//MEM_BARRIER;

	barrier_cross(&barrier);

	if (!thread_id)
    {
		printf("BEFORE size is, %zu\n", (size_t) DS_SIZE(set));
    }

	barrier_cross(&barrier_global);

	while (stop == 0) 
    {
		TEST_LOOP_ONLY_UPDATES();
    }

	barrier_cross(&barrier);

	if (!thread_id)
    {
		size_after = DS_SIZE(set);
		printf("AFTER  size is, %zu\n", size_after);
	}

	barrier_cross(&barrier);

	putting_count[thread_id] += my_putting_count;
	getting_count[thread_id] += my_getting_count;
	removing_count[thread_id]+= my_removing_count;

	putting_count_succ[thread_id] += my_putting_count_succ;
	getting_count_succ[thread_id] += my_getting_count_succ;
	removing_count_succ[thread_id]+= my_removing_count_succ;

	#if GC == 1
		ssmem_term();
		free(alloc);
	#endif
	pthread_exit(NULL);
}

int main(int argc, char **argv) 
{
	set_cpu(0);
	ssalloc_init();
	seeds = seed_rand();

	struct option long_options[] = 
	{
		{"help",                      no_argument,       NULL, 'h'},
		{"duration",                  required_argument, NULL, 'd'},
		{"initial-size",              required_argument, NULL, 'i'},
		{"num-threads",               required_argument, NULL, 'n'},
		{"range",                     required_argument, NULL, 'r'},
		{"update-rate",               required_argument, NULL, 'u'},
		{"put-rate",               	  required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};

	int i, c;
	while(1) 
    {
		i = 0;
		c = getopt_long(argc, argv, "hAf:d:i:n:r:u:p:", long_options, &i);
		
		if(c == -1)
			break;		
		if(c == 0 && long_options[i].flag == 0)
			c = long_options[i].val;
		switch(c) 
		{
			case 0:
			  /* Flag is automatically set */
			  break;
			case 'h':
			  printf("Help information:"
				 "\n"
				 "\n"
				 "Usage:\n"
				 "  %s [options...]\n"
				 "\n"
				 "Options:\n"
				 "  -h, --help\n"
				 "        Print this message\n"
				 "  -d, --duration <int>\n"
				 "        Test duration in milliseconds\n"
				 "  -i, --initial-size <int>\n"
				 "        Number of elements to insert before test\n"
				 "  -n, --num-threads <int>\n"
				 "        Number of threads\n"
				 "  -r, --range <int>\n"
				 "        Range of integer keys inserted in set\n"
				 "  -p, --put-rate <int>\n"
				 "        Percentage of put (insert) operations\n"
				 , argv[0]);
			  exit(0);
			case 'd':
			  duration = atoi(optarg);
			  break;
			case 'i':
			  initial = atoi(optarg);
			  break;
			case 'n':
			  num_threads = atoi(optarg);
			  break;
			case 'r':
			  range = atol(optarg);
			  break;
			case 'u':
			  update = atoi(optarg);
			  break;
			case 'p':
			  put_explicit = 1;
			  put = atoi(optarg);
			  break;
			case '?':
			default:
			  printf("Use -h or --help for help\n");
			  exit(1);
		}
    }

	if (range < initial)
    {
		range = 2 * initial;
    }
  
	printf("Initial, %zu \n", initial);
	printf("Range, %zu \n", range);
	printf("Algorithm, TSLQueue \n");
	if (!is_power_of_two(range))
    {
		size_t range_pow2 = pow2roundup(range);
		printf("** rounding up range (to make it power of 2): old: %zu / new: %zu\n", range, range_pow2);
		range = range_pow2;
    }

	if (put > update)
    {
		put = update;
    }
	update_rate = update / 100.0;
	if (put_explicit)
    {
		put_rate = put / 100.0;
    }
	else
    {
		put_rate = update_rate / 2;
    }
	get_rate = 1 - update_rate;

	rand_max = range - 1;
    
	struct timeval start, end;
	struct timespec timeout;
	timeout.tv_sec = duration / 1000;
	timeout.tv_nsec = (duration % 1000) * 1000000;
    
	stop = 0;

	DS_TYPE* set = DS_NEW(num_threads);
	assert(set != NULL);
	
	printf("Size of node padded, %u B\n", sizeof(DS_NODE));
	double kb = initial * sizeof(DS_NODE) / 1024.0;
	double mb = kb / 1024.0;
	printf("Size of initial, %.2f KB is %.2f MB\n", kb, mb);

  /* Initializes the local data */
	putting_count = (ticks *) calloc(num_threads , sizeof(ticks));
	putting_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));
	getting_count = (ticks *) calloc(num_threads , sizeof(ticks));
	getting_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));
	removing_count = (ticks *) calloc(num_threads , sizeof(ticks));
	removing_count_succ = (ticks *) calloc(num_threads , sizeof(ticks));
    
	pthread_t threads[num_threads];
	pthread_attr_t attr;
	int rc;
	void *status;
    
	barrier_init(&barrier_global, num_threads + 1);
	barrier_init(&barrier, num_threads);
    
	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
	thread_data_t* tds = (thread_data_t*) malloc(num_threads * sizeof(thread_data_t));

	long t;
	for(t = 0; t < num_threads; t++)
    {
		tds[t].id = t;
		tds[t].set = set;
		rc = pthread_create(&threads[t], &attr, test, tds + t);
		if (rc)
		{
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}   
    }
    
	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
    
	barrier_cross(&barrier_global);
	gettimeofday(&start, NULL);
	nanosleep(&timeout, NULL);

	stop = 1;
	gettimeofday(&end, NULL);
	duration = (end.tv_sec * 1000 + end.tv_usec / 1000) - (start.tv_sec * 1000 + start.tv_usec / 1000);
    
	for(t = 0; t < num_threads; t++) 
    {
		rc = pthread_join(threads[t], &status);
		if (rc) 
		{
			printf("ERROR; return code from pthread_join() is %d\n", rc);
			exit(-1);
		}
    }

	free(tds);
    
	volatile uint64_t putting_count_total = 0;
	volatile uint64_t putting_count_total_succ = 0;
	volatile uint64_t getting_count_total = 0;
	volatile uint64_t getting_count_total_succ = 0;
	volatile uint64_t removing_count_total = 0;
	volatile uint64_t removing_count_total_succ = 0;
    
	for(t=0; t < num_threads; t++) 
    {	
		putting_count_total += putting_count[t];
		putting_count_total_succ += putting_count_succ[t];
		getting_count_total += getting_count[t];
		getting_count_total_succ += getting_count_succ[t]; 
		removing_count_total += removing_count[t];
		removing_count_total_succ += removing_count_succ[t];
    }
    
	#define LLU long long unsigned int

	int UNUSED pr = (int) (putting_count_total_succ - removing_count_total_succ);

	#if VALIDATESIZE==1	
		if (size_after != (initial + pr))
		{
			printf("\n******** ERROR WRONG size. %zu + %d != %zu (difference %d)**********\n\n", initial, pr, size_after, (initial + pr)-size_after);
		} 
	#endif
	uint64_t total = putting_count_total + getting_count_total + removing_count_total;
	double putting_perc = 100.0 * (1 - ((double)(total - putting_count_total) / total));
	double putting_perc_succ = (1 - (double) (putting_count_total - putting_count_total_succ) / putting_count_total) * 100;
	double removing_perc = 100.0 * (1 - ((double)(total - removing_count_total) / total));
	double removing_perc_succ = (1 - (double) (removing_count_total - removing_count_total_succ) / removing_count_total) * 100;
	 
	printf("Inserting_count_total , %-10llu \n", (LLU) putting_count_total);
	printf("Inserting_count_total_succ , %-10llu \n", (LLU) putting_count_total_succ);
	printf("Inserting_perc_succ , %10.1f \n", putting_perc_succ);
	printf("Inserting_perc , %10.1f \n", putting_perc);
	printf("Inserting_effective , %10.1f \n", (putting_perc * putting_perc_succ) / 100);
  
	printf("DeleteMin_count_total , %-10llu \n", (LLU) removing_count_total);
	printf("DeleteMin_count_total_succ , %-10llu \n", (LLU) removing_count_total_succ);
	printf("DeleteMin_perc_succ , %10.1f \n", removing_perc_succ);
	printf("DeleteMin_perc , %10.1f \n", removing_perc);
	printf("DeleteMin_effective , %10.1f \n", (removing_perc * removing_perc_succ) / 100);
  
  
	double throughput = (putting_count_total + removing_count_total) * 1000.0 / duration;
  
	printf("Number of Threads , %zu \n", num_threads);
	printf("Million Operations per Second , %.3f\n", throughput / 1e6);
	printf("Operations per Second , %.2f\n", throughput);
    
	pthread_exit(NULL);
	
	return 0;
}
