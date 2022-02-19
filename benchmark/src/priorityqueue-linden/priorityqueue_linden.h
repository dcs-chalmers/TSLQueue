/*   
 *   File: priorityqueue_linden.h
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
#include <assert.h>
#include "utils.h"
#include "common.h"
#include "ssalloc.h"
#include "ssmem.h"

/******************x86 code atomics*****************************/
#define CAS(a,b,c) 			__sync_bool_compare_and_swap(a,b,c)
#define FAXOR(a,b) 			__sync_fetch_and_or(a,b);
/***************************************************************/

#define get_marked_ref(_p)      ((void *)(((uintptr_t)(_p)) | 1))
#define get_unmarked_ref(_p)    ((void *)(((uintptr_t)(_p)) & ~1))
#define is_marked_ref(_p)       (((uintptr_t)(_p)) & 1)


#define DS_ADD(s,k,v)       insert(s,k,v)
#define DS_REMOVE(s)        delete_min(s)
#define DS_SIZE(s)          pq_size(s)
#define DS_LOCAL()          thread_init() 
#define DS_NEW(r)           create_set(r)

#define DS_TYPE             pq_set_t
#define DS_NODE             node_t
#define DS_KEY              skey_t

typedef struct linden_pq_node
{
    skey_t    key;
    int       level;
    int       inserting;
    sval_t    value;
    struct node_s *next[1];
} node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct linden_pq_set
{
    int    max_offset;
    int    max_level;
    int    node_size;
    node_t *head;
    node_t *tail;
    uint8_t padding[CACHE_LINE_SIZE - ((sizeof(node_t*)*2) + (sizeof(int)*3))];
} pq_set_t;

/*Thread local variables*/
extern __thread ssmem_allocator_t* alloc;

/* Interfaces */
pq_set_t *create_set();
void thread_init();
node_t* create_node(pq_set_t *set);
int insert(pq_set_t *set, skey_t key, sval_t value);
sval_t delete_min(pq_set_t *set);
int get_rand_level(pq_set_t *set);
int floor_log_2(unsigned int n);
void restructure(pq_set_t *set);
node_t *locate_preds(pq_set_t *set, skey_t key, node_t ** volatile preds, node_t ** volatile succs);
uint32_t pq_size(pq_set_t *set);

