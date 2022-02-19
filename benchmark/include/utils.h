#ifndef _UTILS_H_INCLUDED_
	#define _UTILS_H_INCLUDED_
	
	#include <stdlib.h>
	#include <stdio.h>
	#include <errno.h>
	#include <string.h>
	#include <sched.h>
	#include <inttypes.h>
	#include <sys/time.h>
	#include <unistd.h>
	#ifdef __sparc__
		#include <sys/types.h>
		#include <sys/processor.h>
		#include <sys/procset.h>
	#elif defined(__tile__)
		#include <arch/atomic.h>
		#include <arch/cycle.h>
		#include <tmc/cpus.h>
		#include <tmc/task.h>
		#include <tmc/spin.h>
		#include <sched.h>
	#else
		#if defined(PLATFORM_MCORE)
			#include <numa.h>
		#endif
		#if defined(__SSE__)
			#include <xmmintrin.h>
			#else
			#define _mm_pause() asm volatile ("nop")
		#endif
		#if defined(__SSE2__)
			#include <emmintrin.h>
		#endif
	#endif
	#include <pthread.h>
	#include "getticks.h"
	#include "random.h"
	#include "ssalloc.h"
	
	
	#ifdef __cplusplus 
		extern "C" 
		{
	#endif
			
			#define DO_ALIGN
			
			#if !defined(false)
				#define false 0
			#endif
			
			#if !defined(true)
				#define true 1
			#endif
			
			#define likely(x)       __builtin_expect((x), 1)
			#define unlikely(x)     __builtin_expect((x), 0)
			
			
			#if !defined(UNUSED)
				#define UNUSED __attribute__ ((unused))
			#endif
			
			#if defined(DO_ALIGN)
				#define ALIGNED(N) __attribute__ ((aligned (N)))
				#else
				#define ALIGNED(N)
			#endif
			
			#if !defined(COMPILER_BARRIER)
				#define COMPILER_BARRIER() asm volatile ("" ::: "memory")
			#endif
			
			#if !defined(COMPILER_NO_REORDER)
				#define COMPILER_NO_REORDER(exec)		\
				COMPILER_BARRIER();				\
				exec;						\
				COMPILER_BARRIER()
			#endif
			
			/***************************** Machine dependent thread affinity **************************************************/
			#if defined(DEFAULT)
				#define NUMBER_OF_SOCKETS 1
				#define CORES_PER_SOCKET  CORE_NUM
				#define CACHE_LINE_SIZE   64
				#define NOP_DURATION      2
				#if AFFINITY == 2 //alternate sockets (sparse pinning)
					static uint8_t __attribute__ ((unused)) the_cores[] ={
						0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15,16,24,17,25,18,26,19,27,20,28,21,29,22,30,23,31
					};
					#else
					#if HYPERTHREAD == 1
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23,8,24,9,25,10,26,11,27,12,28,13,29,14,30,15,31 
						};
						#else
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71 
						};
					#endif
				#endif
			#endif  /*  */
			
			#if defined(EXCESS)
				#define NUMBER_OF_SOCKETS 2
				#define CORES_PER_SOCKET  16
				#define CACHE_LINE_SIZE   64
				#define NOP_DURATION      2
				#if AFFINITY == 2 	//alternate sockets (sparse pinning)
					static uint8_t __attribute__ ((unused)) the_cores[] ={
						0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15,16,24,17,25,18,26,19,27,20,28,21,29,22,30,23,31
					};
					#else
					#if HYPERTHREAD == 1 	//dense pinning fill one core of the same socket at a time with hyperthreading
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,16,1,17,2,18,3,19,4,20,5,21,6,22,7,23,8,24,9,25,10,26,11,27,12,28,13,29,14,30,15,31 
						};
						#else 	//dense pinning one thread per core of the same socket at a time without hyperthreading
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 
						};
					#endif
				#endif
			#endif  /* adon- */
			
			#if defined(ODYSSEUS)
				#define NUMBER_OF_SOCKETS 1
				#define CORES_PER_SOCKET  CORE_NUM
				#define CACHE_LINE_SIZE   64
				#define NOP_DURATION      2
				#if AFFINITY == 2 	//alternate tiles (sparse pinning)
					static uint32_t __attribute__ ((unused)) the_cores[] ={
						0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,
						1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71,
						72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,128,130,132,134,136,138,140,142,
						73,75,77,79,81,83,85,87,89,91,93,95,97,99,101,103,105,107,109,111,113,115,117,119,121,123,125,127,129,131,133,135,137,139,141,143,
						144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,192,194,196,198,200,202,204,206,208,210,212,214,
						145,147,149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,181,183,185,187,189,191,193,195,197,199,201,203,205,207,209,211,213,215,
						216,218,220,222,224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,256,258,260,262,264,266,268,270,272,274,276,278,280,282,284,286,
						217,219,221,223,225,227,229,231,233,235,237,239,241,243,245,247,249,251,253,255,257,259,261,263,265,267,269,271,273,275,277,279,281,283,285,287
					};
					#else					 
					#if HYPERTHREAD == 1 	//dense pinning fill one tile at a time with hyperthreading
						static uint32_t __attribute__ ((unused)) the_cores[] ={
							0,72,144,216,1,73,145,217,2,74,146,218,3,75,147,219,4,76,148,220,5,77,149,221,6,78,150,222,7,79,151,223,8,80,152,224,9,81,153,225,10,82,154,226,11,83,155,227,12,84,156,228,13,85,157,229,14,86,158,230,15,87,159,231,16,88,160,232,17,89,161,233,18,90,162,234,19,91,163,235,20,92,164,236,21,93,165,237,22,94,166,238,23,95,167,239,24,96,168,240,25,97,169,241,26,98,170,242,27,99,171,243,28,100,172,244,29,101,173,245,30,102,174,246,31,103,175,247,32,104,176,248,33,105,177,249,34,106,178,250,35,107,179,251,36,108,180,252,37,109,181,253,38,110,182,254,39,111,183,255,40,112,184,256,41,113,185,257,42,114,186,258,43,115,187,259,44,116,188,260,45,117,189,261,46,118,190,262,47,119,191,263,48,120,192,264,49,121,193,265,50,122,194,266,51,123,195,267,52,124,196,268,53,125,197,269,54,126,198,270,55,127,199,271,56,128,200,272,57,129,201,273,58,130,202,274,59,131,203,275,60,132,204,276,61,133,205,277,62,134,206,278,63,135,207,279,64,136,208,280,65,137,209,281,66,138,210,282,67,139,211,283,68,140,212,284,69,141,213,285,70,142,214,286,71,143,215,287
						};
						#else 	//dense pinning fill one tile at a time without hyperthreading
						static uint32_t __attribute__ ((unused)) the_cores[] ={
							0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,
							72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
							144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,
							216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,256,257,258,259,260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275,276,277,278,279,280,281,282,283,284,285,286,287
						};
					#endif
				#endif
			#endif   
			#if defined(ITHACA)
				#define NUMBER_OF_SOCKETS 2
				#define CORES_PER_SOCKET  36
				#define CACHE_LINE_SIZE   64
				#define NOP_DURATION      2
				#if AFFINITY == 2 	//alternate sockets (sparse pinning)
					static uint8_t __attribute__ ((unused)) the_cores[] ={
						0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
						36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71
					};
					#else
					#if HYPERTHREAD == 1	//dense pinning fill one core of the same socket at a time with hyperthreading
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,36,2,38,4,40,6,42,8,44,10,46,12,48,14,50,16,52,18,54,20,56,22,58,24,60,26,62,28,64,30,66,32,68,34,70,
							1,37,3,39,5,41,7,43,9,45,11,47,13,49,15,51,17,53,19,55,21,57,23,59,25,61,27,63,29,65,31,67,33,69,35,71
						};
						#else	//dense pinning one thread per core of the same socket at a time without hyperthreading
						static uint8_t __attribute__ ((unused)) the_cores[] ={
							0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,
							1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,
							36,38,40,42,44,46,48,50,52,54,56,58,60,62,64,66,68,70,
							37,39,41,43,45,47,49,51,53,55,57,59,61,63,65,67,69,71
						};
					#endif
				#endif
			#endif  
			/*************************************************************************************************************************************/
			
			static inline int
			is_power_of_two (unsigned int x) 
			{
				return ((x != 0) && !(x & (x - 1)));
			}
			
			
			#ifdef __sparc__
				#define PAUSE    asm volatile("rd    %%ccr, %%g0\n\t" ::: "memory")
				#elif defined(__tile__)
				#define PAUSE cycle_relax()
				#else
				#define PAUSE _mm_pause()
			#endif
			
			static inline void pause_rep(uint32_t num_reps)
			{
				volatile uint32_t i;
				for (i = 0; i < num_reps; i++)
				{
					PAUSE;
					/* PAUSE; */
					/* asm volatile ("NOP"); */
				}
			}
			
			static inline void nop_rep(uint32_t num_reps)
			{
				uint32_t i;
				for (i = 0; i < num_reps; i++)
				{
					asm volatile ("");
				}
			}
			
			/* PLATFORM specific -------------------------------------------------------------------- */
			#if !defined(PREFETCHW)
				#if defined(__x86_64__)
					#  define PREFETCHW(x)		     asm volatile("prefetchw %0" :: "m" (*(unsigned long *)x))
					#elif defined(__sparc__)
					#  define PREFETCHW(x)		
					#elif defined(XEON)
					#  define PREFETCHW(x)		
					#else
					#  define PREFETCHW(x)		
				#endif
			#endif 
			
			#if !defined(PREFETCH)
				#if defined(__x86_64__)
					#  define PREFETCH(x)		     asm volatile("prefetch %0" :: "m" (*(unsigned long *)x))
					#elif defined(__sparc__)
					#  define PREFETCH(x)		
					#elif defined(XEON)
					#  define PREFETCH(x)		
					#else
					#  define PREFETCH(x)		
				#endif
			#endif 
			
			
			//debugging functions
			#ifdef DEBUG
				#define DPRINT(args...) fprintf(stderr,args);
				#define DDPRINT(fmt, args...) printf("%s:%s:%d: "fmt, __FILE__, __FUNCTION__, __LINE__, args)
				#else
				#define DPRINT(...)
				#define DDPRINT(fmt, ...)
			#endif
			
			
			
			static inline int get_cluster(int thread_id) {
				#ifdef __solaris__
					if (thread_id>64){
						perror("Thread id too high");
						return 0;
					}
					return thread_id/CORES_PER_SOCKET;
					//    return the_sockets[thread_id];
					#elif XEON
					if (thread_id>=80){
						perror("Thread id too high");
						return 0;
					}
					
					return the_sockets[thread_id];
					#elif defined(__tile__)
					return 0;
					#else
					return thread_id/CORES_PER_SOCKET;
				#endif
			}
			
			static inline double wtime(void)
			{
				struct timeval t;
				gettimeofday(&t,NULL);
				return (double)t.tv_sec + ((double)t.tv_usec)/1000000.0;
			}
			
			static inline 
			void set_cpu(int cpu) 
			{
				#ifndef NO_SET_CPU
					#ifdef __sparc__
						processor_bind(P_LWPID,P_MYID, the_cores[cpu], NULL);
						#elif defined(__tile__)
						if (cpu>=tmc_cpus_grid_total()) 
						{
							perror("Thread id too high");
						}
						// cput_set_t cpus;
						if (tmc_cpus_set_my_cpu(the_cores[cpu])<0) 
						{
							tmc_task_die("tmc_cpus_set_my_cpu() failed."); 
						}    
						#else
						int n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
						//    cpu %= (NUMBER_OF_SOCKETS * CORES_PER_SOCKET);
						if (cpu < n_cpus)
						{
							int cpu_use = the_cores[cpu];
							cpu_set_t mask;
							CPU_ZERO(&mask);
							CPU_SET(cpu_use, &mask);
							#  if defined(PLATFORM_NUMA)
								numa_set_preferred(get_cluster(cpu_use));
							#  endif
							pthread_t thread = pthread_self();
							if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &mask) != 0) 
							{
								fprintf(stderr, "Error setting thread affinity\n");
							}
						}
					#endif
				#endif
			}
			
			
			static inline void cdelay(ticks cycles)
			{
				if (unlikely(cycles == 0))
				{
					return;
				}
				ticks __ts_end = getticks() + (ticks) cycles;
				while (getticks() < __ts_end);
			}
			
			static inline void cpause(ticks cycles)
			{
				#if defined(XEON)
					cycles >>= 3;
					ticks i;
					for (i=0;i<cycles;i++) 
					{
						_mm_pause();
					}
					#else
					ticks i;
					for (i=0;i<cycles;i++) 
					{
						__asm__ __volatile__("nop");
					}
				#endif
			}
			
			static inline void udelay(unsigned int micros)
			{
				double __ts_end = wtime() + ((double) micros / 1000000);
				while (wtime() < __ts_end);
			}
			
			//getticks needs to have a correction because the call itself takes a
			//significant number of cycles and skewes the measurement
			extern ticks getticks_correction_calc();
			
			static inline ticks get_noop_duration() {
				#define NOOP_CALC_REPS 1000000
				ticks noop_dur = 0;
				uint32_t i;
				ticks corr = getticks_correction_calc();
				ticks start;
				ticks end;
				start = getticks();
				for (i=0;i<NOOP_CALC_REPS;i++) {
					__asm__ __volatile__("nop");
				}
				end = getticks();
				noop_dur = (ticks)((end-start-corr)/(double)NOOP_CALC_REPS);
				return noop_dur;
			}
			
			/// Round up to next higher power of 2 (return x if it's already a power
			/// of 2) for 32-bit numbers
			static inline uint32_t pow2roundup (uint32_t x)
			{
				if (x==0) return 1;
				--x;
				x |= x >> 1;
				x |= x >> 2;
				x |= x >> 4;
				x |= x >> 8;
				x |= x >> 16;
				return x+1;
			}
			
			static const size_t pause_fixed = 16384;
			
			static inline void do_pause()
			{
				cpause((mrand(seeds) % pause_fixed));
			}
			
			
			static const size_t pause_max   = 16384;
			static const size_t pause_base  = 512; //64;//ad-
			static const size_t pause_min   = 512; //64;//ad-
			
			static inline void do_pause_exp(size_t nf)
			{
				if (unlikely(nf > 32))
				{
					nf = 32;
				}
				const size_t p = (pause_base << nf);
				const size_t pm = (p > pause_max) ? pause_max : p;
				const size_t tp = pause_min + (mrand(seeds) % pm);
				
				cdelay(tp);
			}
			
			// 0: fixed max pause
			// 1: exponentially increasing pause
			#define DO_PAUSE_EXP(nr)            do_pause_exp(nr);//cdelay(nr); // //ruk
			#define DO_PAUSE_TYPE     1       
			#if DO_PAUSE_TYPE == 0
				#define DO_PAUSE()            do_pause()
				#define NUM_RETRIES()        
				#elif DO_PAUSE_TYPE == 1
				#define DO_PAUSE()            do_pause_exp(__nr++);
				#define NUM_RETRIES()         UNUSED size_t __nr;
				#else
			#endif
			
	#ifdef __cplusplus
		}		
	#endif
	
	
#endif
