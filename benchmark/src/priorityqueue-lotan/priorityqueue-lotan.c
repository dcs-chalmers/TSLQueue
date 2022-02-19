/*   
 *   File: priorityqueue-lotan.c
 *   Author: Adones Rukundo 
 *	 <adones@chalmers.se, adones@must.ac.ug, adon.rkd@gmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "priorityqueue-lotan.h"
void thread_init()
{
	
}
pq_set_t *create_set(size_t range)
{
    int i;
	volatile pq_set_t *set;
	int max_level = floor_log_2((unsigned int) range);
	int node_size = sizeof(node_t) + (max_level * sizeof(node_t *));
	while (node_size & 31)
    {
		node_size++;
	}
	
	if ((set = (volatile pq_set_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(pq_set_t))) == NULL)
    {
		perror("malloc");
		exit(1);
	}
    	
	set->head=(volatile node_t *)ssalloc(node_size);
    set->tail=(volatile node_t *)ssalloc(node_size);
	
    set->head->key=0;
    set->head->level=max_level;
    set->head->value=NULL;
	
    set->tail->key=range+1;
    set->tail->level=max_level;
    set->tail->value=NULL;
	
    for(i=0;i<max_level;i++) 
	{
		set->head->next[i]=set->tail;
		set->tail->next[i]=NULL;
	}
	
	set->max_level=max_level;
	set->node_size=node_size;
	
    return set;
}

node_t* create_node(pq_set_t *set, skey_t key, sval_t value, int level)
{
    node_t *node;
    
    #if GC == 1
        size_t ns = set->node_size;
        node = (node_t*) ssmem_alloc(alloc, ns);
    #else
        size_t ns = set->node_size;
        node = (node_t *)ssalloc(ns);
    #endif
    
    if (node == NULL)
    {
        perror("malloc");
        exit(1);
    }
    
    node->key = key;
    node->value = value;
    node->level = level;
    node->deleted = 0;
       
    return node;
}

int insert(pq_set_t *set, skey_t key, sval_t value)
{
	int result = 0;	
	result = fraser_insert(set, key, value);
	return result;
}

sval_t delete_min(pq_set_t *set)
{
	sval_t result = 0;
	node_t *node;
	node = GET_UNMARKED(set->head->next[0]);

	while(node->next[0]!=NULL)
	{
		if(result=fraser_remove(set, node->key))
			break; //memory reclamation
		node = GET_UNMARKED(node->next[0]);
	}
	
	return result;
}

int fraser_insert(pq_set_t *set, skey_t key, sval_t value) 
{
	node_t *new, *pred, *succ;
	node_t *succs[set->max_level], *preds[set->max_level];
	int i, found;
	
	retry:
	found = fraser_search_no_cleanup(set, key, preds, succs);	
	if(found)
    {
		return false;
	}	
	new = create_node(set, key, value, get_rand_level(set));
	
	for (i = 0; i < new->level; i++)
    {
		new->next[i] = succs[i];
	}

	/* Node is visible once inserted at lowest level */
	if (!CAS(&preds[0]->next[0], GET_UNMARKED(succs[0]), new))
    {
		#if GC == 1
			ssmem_free(alloc, (void*) new);
        #else
			ssfree(new);
		#endif
		goto retry;
	}
	
	for (i = 1; i < new->level; i++) 
    {
		while (1) 
		{
			pred = preds[i];
			succ = succs[i];
			if (IS_MARKED(new->next[i]))
			{
				return true;
			}
			if (CAS(&pred->next[i], succ, new)) break;
			fraser_search(set, key, preds, succs);
		}
	}
	return true;
}

sval_t fraser_remove(pq_set_t *set, skey_t key)
{	
	node_t* succs[set->max_level];
	sval_t result = 0;
	
	int found = fraser_search_no_cleanup_succs(set, key, succs);
	
	if (!found)
    {
		return false;
	}
	
	node_t* node_del = succs[0];
	int my_delete = mark_node_ptrs(node_del);
	
	if (my_delete)
    {
		result = node_del->value;
		fraser_search(set, key, NULL, NULL);
		#if GC == 1
			ssmem_free(alloc, (void*) succs[0]);
		#endif
	}
	return result;
}

int	fraser_search(pq_set_t *set, skey_t key, node_t **left_list, node_t **right_list)
{
	int i; 
	node_t *left, *left_next, *right = NULL, *right_next;
	
	retry:
	left = set->head;
	for (i = set->max_level - 1; i >= 0; i--)
    {
		left_next = left->next[i];
		if ((is_marked((uintptr_t)left_next)))
		{
			goto retry;
		}
		/* Find unmarked node pair at this level */
		for (right = left_next; ; right = right_next)
		{
			/* Skip a sequence of marked nodes */
			right_next = right->next[i];
			while ((is_marked((uintptr_t)right_next)))
			{
				right = (node_t*)unset_mark((uintptr_t)right_next);
				right_next = right->next[i];
			}
			
			if (right->key >= key)
			{
				break;
			}
			left = right; 
			left_next = right_next;
		}
		/* Ensure left and right nodes are adjacent */
		if ((left_next != right))
		{
			if(!CAS(&left->next[i], left_next, right))
			{
				goto retry;
			}
		}
		
		if (left_list != NULL)
      	{
			left_list[i] = left;
		}
		if (right_list != NULL)
      	{
			right_list[i] = right;
		}
	}
	return (right->key == key);
}

int fraser_search_no_cleanup(pq_set_t *set, skey_t key, node_t **left_list, node_t **right_list)
{
	int i;
	node_t *left, *left_next, *right = NULL;
	
	left = set->head;
	for (i = set->max_level - 1; i >= 0; i--)
    {
		left_next = GET_UNMARKED(left->next[i]);
		right = left_next;
		while (1)
		{			
			if (likely(!IS_MARKED(right->next[i])))
			{
				if (right->key >= key)
				{
					break;
				}
				left = right;
			}
			right = GET_UNMARKED(right->next[i]);
		}
		
		left_list[i] = left;
		right_list[i] = right;
	}
	return (right->key == key);
}

int fraser_search_no_cleanup_succs(pq_set_t *set, skey_t key, node_t **right_list)
{
	int i;
	node_t *left, *left_next, *right = NULL;
	
	left = set->head;
	for (i = set->max_level - 1; i >= 0; i--)
    {
		left_next = GET_UNMARKED(left->next[i]);
		right = left_next;
		while (1)
		{
			if (likely(!IS_MARKED(right->next[i])))
			{
				if (right->key >= key)
				{
					break;
				}
				left = right;
			}
			right = GET_UNMARKED(right->next[i]);
		}		
		right_list[i] = right;
	}
	return (right->key == key);
}

int mark_node_ptrs(node_t *node)
{
	int i, cas = 0;
	node_t* n_next;
	
	for (i = node->level - 1; i >= 0; i--)
    {
		do
      	{
			n_next = node->next[i];
			if (is_marked((uintptr_t)n_next))
      	    {
				cas = 0;
				break;
			}
			cas = CAS(&node->next[i], GET_UNMARKED(n_next), set_mark((uintptr_t)n_next));
		} 
		while (!cas);
	}
	return (cas);
}

int pq_size(pq_set_t *set)
{
    int size = 0;
    node_t *node;
    
    node = GET_UNMARKED(set->head->next[0]);
    while (node->next[0] != NULL)
    {
        if (!IS_MARKED(node->next[0]))
        {
            size++;
        }
        node = GET_UNMARKED(node->next[0]);
    } 
    return size;
}

int get_rand_level(pq_set_t *set)
{
    int i, level = 1;
    for (i = 0; i < set->max_level - 1; i++)
    {
        if ((rand_range(101)) < 50)
          level++;
        else
          break;
    }
    
    return level;
}

int floor_log_2(unsigned int n)
{
    int pos = 0;
    if (n >= 1<<16) { n >>= 16; pos += 16; }
    if (n >= 1<< 8) { n >>=  8; pos +=  8; }
    if (n >= 1<< 4) { n >>=  4; pos +=  4; }
    if (n >= 1<< 2) { n >>=  2; pos +=  2; }
    if (n >= 1<< 1) {           pos +=  1; }
    return ((n == 0) ? (-1) : pos);
}
