/*   
 *   File: priorityqueue-lotan.h
 *   Author: Adones Rukundo 
 *	 <adones@chalmers.se, adones@must.ac.ug, adon.rkd@gmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "common.h"
#include "ssalloc.h"
#include "ssmem.h"

/******************x86 code atomics*****************************/
#define CAS(a,b,c) 			__sync_bool_compare_and_swap(a,b,c)
/***************************************************************/

#define DS_ADD(s,k,t)       insert(s, k, k)
#define DS_REMOVE(s)        delete_min(s)
#define DS_SIZE(s)          pq_size(s)
#define DS_LOCAL()          thread_init() 
#define DS_NEW(r)           create_set(r)

#define DS_TYPE             pq_set_t
#define DS_NODE             node_t
#define DS_KEY              skey_t

typedef intptr_t level_t;

typedef volatile struct sl_node
{
  skey_t key;
  sval_t value;
  uint32_t deleted;
  uint32_t level;
  volatile struct sl_node* next[1];
} node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct sl_set
{
    int    max_level;
    int    node_size;
    node_t *head;
    node_t *tail;
    uint8_t padding[CACHE_LINE_SIZE - ((sizeof(node_t*)*2) + (sizeof(int)*2))];
} pq_set_t;

/*Thread local variables*/
extern __thread ssmem_allocator_t* alloc;

/* Interfaces */
static inline int is_marked(uintptr_t i)
{
  return (int)(i & (uintptr_t)0x01);
}

static inline uintptr_t unset_mark(uintptr_t i)
{
  return (i & ~(uintptr_t)0x01);
}

static inline uintptr_t set_mark(uintptr_t i)
{
  return (i | (uintptr_t)0x01);
}

#define GET_UNMARKED(p) (node_t*) unset_mark((uintptr_t) (p))
#define GET_MARKED(p) set_mark((uintptr_t) (p))
#define IS_MARKED(p) is_marked((uintptr_t) (p))

pq_set_t *create_set(size_t range);
void thread_init();
node_t* create_node(pq_set_t *set, skey_t key, sval_t value, int level);
int insert(pq_set_t *set, skey_t key, sval_t value);
sval_t delete_min(pq_set_t *set);
int pq_size(pq_set_t *set);
int mark_node_ptrs(node_t *node);
sval_t fraser_find(pq_set_t *set, skey_t key);
sval_t fraser_remove(pq_set_t *set, skey_t key);
int fraser_insert(pq_set_t *set, skey_t key, sval_t value);
int fraser_search_no_cleanup(pq_set_t *set, skey_t key, node_t **left_list, node_t **right_list);
int fraser_search_no_cleanup_succs(pq_set_t *set, skey_t key, node_t **right_list);
int	fraser_search(pq_set_t *set, skey_t key, node_t **left_list, node_t **right_list);
int get_rand_level(pq_set_t *set);
int floor_log_2(unsigned int n);