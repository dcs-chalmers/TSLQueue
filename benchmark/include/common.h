#ifndef _COMMON_H_
	#define _COMMON_H_
	#include <limits.h>
	#include <string.h>

	#include "getticks.h"
	#include "barrier.h"

	#define XSTR(s)       STR(s)
	#define STR(s)         #s

#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6
		#define STATIC_ASSERT(a, msg)  _Static_assert ((a), msg);
	#else 
		#define STATIC_ASSERT(a, msg)           
	#endif

	#define STRING_LENGTH  8

	typedef intptr_t skey_t;
	typedef intptr_t sval_t;

	typedef struct strkey_t 
	{
		char key[STRING_LENGTH]; 
	} strkey_t;

	typedef struct strval_t 
	{
		char val[STRING_LENGTH];
	} strval_t;
#endif	/*  _COMMON_H_ */

#define DEFAULT_DURATION          1000
#define DEFAULT_INITIAL           12000
#define DEFAULT_NB_THREADS        1
#define DEFAULT_RANGE             1073741824 /*2^30*/
#define DEFAULT_UPDATE            100

#ifndef SIDEWORK
	#define SIDEWORK 0
#endif
	
/********************************** Thread test loop  *********************************/
#define TEST_LOOP_ONLY_UPDATES()														\
	c = (uint32_t)(my_random(&(seeds[0]),&(seeds[1]),&(seeds[2])));						\
	if (unlikely(c < scale_put))														\
	{																					\
		key = (c & rand_max) + rand_min;												\
		int res;																		\
		res = DS_ADD(set, key, key);													\
		if(res)																			\
		{																				\
			my_putting_count_succ++;													\
		}																				\
	  my_putting_count++;																\
	}																					\
	else 																				\
	{																					\
		int removed;																	\
		removed = DS_REMOVE(set);														\
		if(removed != 0)																\
		{																				\
			my_removing_count_succ++;													\
		}																				\
		my_removing_count++;															\
	} 																					\
	cpause(SIDEWORK);
