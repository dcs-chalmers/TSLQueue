/*   
 *   File: priorityqueue-tslqueue.h
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
#define CAE(a,b,c) 			__atomic_compare_exchange (a,b,c,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)
#define CAS(a,b,c) 			__sync_bool_compare_and_swap(a,b,c)
#define FAXOR(a,b) 			__sync_fetch_and_or(a,b);
/***************************************************************/

#define DS_ADD(s,k,t)       	insert(s,k,k)
#define DS_REMOVE(s)    		delete_min(s)
#define DS_SIZE(s)          	pq_size(s)
#define DS_NEW(n)            	create_set(n)
#define DS_LOCAL()          	init_local()

#define DS_TYPE             tsl_set_t
#define DS_NODE             node_t
#define DS_KEY              skey_t

#define LEFT_DIRECTION 1
#define RIGHT_DIRECTION 2
#define DUPLICATE_DIRECTION 3
#define NOT_MARKED 0
#define DELETE_MARK 1
#define INSERT_MARK 2
#define LEAF_MARK 3


/*********MACROS**********/
#define READ_LEFT()									\
	operation_mark = GETMARK(parent_node->next);	\
	parent_node_left= parent_node->left;			\
	child_node = ADDRESS(parent_node_left);			\
	child_mark = GETMARK(parent_node_left);			\
	parent_direction = LEFT_DIRECTION;				
	
#define READ_RIGHT()								\
	operation_mark = GETMARK(parent_node->next);	\
	parent_node_right = parent_node->right;			\
	child_node = ADDRESS(parent_node_right);		\
	child_mark = GETMARK(parent_node_right);		\
	parent_direction = RIGHT_DIRECTION;				

#define TRAVERSE()									\
	if(key <= parent_node->key) 					\
	{												\
		READ_LEFT();								\
	}												\
	else											\
	{												\
		READ_RIGHT();								\
	}																						

typedef ALIGNED(CACHE_LINE_SIZE) struct generic_node
{
	void *parent;
	void *left;
	void *next;
	void *right;
	sval_t value;
	skey_t key;
	
	uint8_t inserting;
	uint8_t parent_direction;
	uint8_t padding[CACHE_LINE_SIZE - (sizeof(skey_t) + sizeof(sval_t) + (sizeof(void*)*4) + (sizeof(uint8_t)*2))];
}node_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct tslqueue_set
{
  node_t *head;
  node_t *root;
  size_t num_threads;
  uint32_t del_scale;
  uint8_t padding[CACHE_LINE_SIZE - ((sizeof(node_t*)*2) + sizeof(size_t) + sizeof(uint32_t))];
} tsl_set_t;

typedef struct next_cast
{
	void *next;
	void *right;
} next_t;

typedef ALIGNED(CACHE_LINE_SIZE) struct insert_seek_record_info
{
    node_t* child;
	node_t* next_node;
	node_t* cas1_node;
	node_t* cas2_node;
	uint8_t duplicate;
	uint8_t parent_direction;
	uint8_t padding[CACHE_LINE_SIZE - ((sizeof(node_t*)*4) + (sizeof(uint8_t)*2))];
} insert_seek_t;

/*Thread local variables*/
extern __thread ssmem_allocator_t* alloc;

/* Interfaces */

static inline node_t* ADDRESS(volatile node_t* ptr) {
    return (node_t*) (((uint64_t)ptr) & 0xfffffffffffffffc);
}
static inline uint64_t GETMARK(volatile node_t* ptr) {
    return ((uint64_t)ptr) & 3;
}

static inline uint64_t MARK_DELETE_NODE(volatile node_t* ptr) {
    return (((uint64_t)ptr)) | DELETE_MARK;
}

static inline uint64_t MARK_INSERT_NODE(volatile node_t* ptr) {
    return (((uint64_t)ptr)) | INSERT_MARK;
}

static inline uint64_t MARK_LEAF_NODE(volatile node_t* ptr) {
    return (((uint64_t)ptr)) | LEAF_MARK;
}

tsl_set_t* create_set(size_t num_threads);
node_t* create_node();
void init_local();

uint8_t insert(tsl_set_t *set, skey_t key, sval_t value);
sval_t delete_min(tsl_set_t *set);
void physical_delete(tsl_set_t *set, volatile node_t *dummy_node);

insert_seek_t insert_search(tsl_set_t *set, skey_t key);
static inline void try_helping_insert(volatile node_t* item_node);

uint32_t pq_size(tsl_set_t *set);
static inline uint32_t random_gen(tsl_set_t *set);