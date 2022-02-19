/*   
 *   File: priorityqueue_linden.c
 *   Author: Adones Rukundo 
 *	 <adones@chalmers.se, adones@must.ac.ug, adon.rkd@gmail.com>
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "priorityqueue_linden.h"

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
	set->head->inserting=0;
	
    set->tail->key=range+1;
    set->tail->level=max_level;
    set->tail->value=NULL;
	set->tail->inserting=0;
	
    for(i=0;i<max_level;i++) 
	{
		set->head->next[i]=set->tail;
	}
	
	set->max_offset=0;
	set->max_level=max_level;
	set->node_size=node_size;
	
    return set;
}

/* initialize new node */
node_t* create_node(pq_set_t *set)
{
    node_t *node;
    
    #if GC == 1
        size_t ns = set->node_size;
        node = (node_t*)ssmem_alloc(alloc, ns);
    #else
        /* use levelmax instead of toplevel in order to be able to use the ssalloc allocator*/
        size_t ns = set->node_size;
        node = (node_t *)ssalloc(ns);
    #endif
    
    if (node == NULL)
    {
        perror("malloc");
        exit(1);
    }
    
    return node;
}

node_t * locate_preds(pq_set_t *set, skey_t key, node_t ** volatile preds, node_t ** volatile succs)
{
    node_t *x, *x_next, *del = NULL;
    int d = 0, i;

    x = set->head;
    i = set->max_level - 1;
    while (i >= 0)
    {
        x_next = x->next[i];
        d = is_marked_ref(x_next);
        x_next = get_unmarked_ref(x_next);
        assert(x_next != NULL);
		
        while (x_next->key < key || is_marked_ref(x_next->next[0]) || ((i == 0) && d)) 
        {
			/* Record bottom level deleted node not having delete flag
             * set, if traversed. */
            if (i == 0 && d)
                del = x_next;
            x = x_next;
            x_next = x->next[i];
            d = is_marked_ref(x_next);
            x_next = get_unmarked_ref(x_next);
            assert(x_next != NULL);
        }
        preds[i] = x;
        succs[i] = x_next;
        i--;
    }
    return del;
}

int insert(pq_set_t *set, skey_t key, sval_t value)
{
    node_t *preds[set->max_level], *succs[set->max_level];
    volatile node_t *new = NULL, *del = NULL;
    
    /* Initialise a new node for insertion. */
    new = create_node(set);
    new->key = key;
    new->value = value;
	new->inserting = 1;
    new->level = get_rand_level(set);

    /* lowest level insertion retry loop */
 retry:
    del = locate_preds(set, key, preds, succs);

    /* return if key already exists, i.e., is present in a non-deleted
     * node */
    if (succs[0]->key == key && !is_marked_ref(preds[0]->next[0]) && preds[0]->next[0] == succs[0]) 
    {
        new->inserting = 0;
        //free_node(new);
        #if GC == 1
			ssmem_free(alloc, (void*) new);
		#endif
        //goto out;
		return 0;
    }
    new->next[0] = succs[0];

    /* The node is logically inserted once it is present at the bottom
     * level. */
    if (!CAS(&preds[0]->next[0], succs[0], new)) 
    {
        /* either succ has been deleted (modifying preds[0]),
         * or another insert has succeeded or preds[0] is head,
         * and a restructure operation has updated it */
        goto retry;
    }

    /* Insert at each of the other levels in turn. */
    int i = 1;
    while ( i < new->level)
    {
        /* If successor of new is deleted, we're done. (We're done if
         * only new is deleted as well, but this we can't tell) If a
         * candidate successor at any level is deleted, we consider
         * the operation completed. */
        if (is_marked_ref(new->next[0]) || is_marked_ref(succs[i]->next[0]) || del == succs[i])
            goto success;

        /* prepare next pointer of new node */
        new->next[i] = succs[i];
        if (!CAS(&preds[i]->next[i], succs[i], new))
        {
			/* failed due to competing insert or restructure */
            del = locate_preds(set, key, preds, succs);

            /* if new has been deleted, we're done */
            if (succs[0] != new) 
                goto success;
        } 
        else 
        {
            /* Succeeded at this level. */
            i++;
        }
    }
 success:
    if (new) 
    {
        /* this flag must be reset *after* all CAS have completed */
        new->inserting = 0;
    }
	return 1;
}

sval_t delete_min(pq_set_t *set)
{
    sval_t   value = NULL;
    node_t *x, *nxt, *obs_head = NULL, *newhead, *cur, *head;
    int offset, lvl;
	
    newhead = NULL;
    offset = lvl = 0;

    x = head = set->head;
    obs_head = x->next[0];

    do 
	{
        
		offset++;
		
        /* expensive, high probability that this cache line has
         * been modified */
        nxt = x->next[0];

        // tail cannot be deleted
        if (get_unmarked_ref(nxt) == set->tail) 
		{
			return 0;
        }

        /* Do not allow head to point past a node currently being
         * inserted. This makes the lock-freedom quite a theoretic
         * matter. */
        if (newhead == NULL && x->inserting) newhead = x;

        /* optimization */
        if (is_marked_ref(nxt)) continue;
        /* the marker is on the preceding pointer */
        /* linearisation point deletemin */
        nxt = FAXOR(&x->next[0], 1);
    }
    while ( (x = get_unmarked_ref(nxt)) && is_marked_ref(nxt) );

    assert(!is_marked_ref(x));

    value = x->value;

    
    /* If no inserting node was traversed, then use the latest 
     * deleted node as the new lowest-level head pointed node
     * candidate. */
    if (newhead == NULL) newhead = x;

    /* if the offset is big enough, try to update the head node and
     * perform memory reclamation */
    if (offset < set->max_offset) goto out;

    /* Optimization. Marginally faster */
    if (head->next[0] != obs_head) goto out;
    
    /* try to swing the lowest level head pointer to point to newhead,
     * which is deleted */
    if (CAS(&head->next[0], obs_head, get_marked_ref(newhead)))
    {
        /* Update higher level pointers. */
        restructure(set);

        /* We successfully swung the upper head pointer. The nodes
         * between the observed head (obs_head) and the new bottom
         * level head pointed node (newhead) are guaranteed to be
         * non-live. Mark them for recycling. */
        cur = get_unmarked_ref(obs_head);
        while (cur != get_unmarked_ref(newhead)) 
		{
            nxt = get_unmarked_ref(cur->next[0]);
            assert(is_marked_ref(cur->next[0]));
			#if GC == 1
				ssmem_free(alloc, (void*) cur);
			#endif
            cur = nxt;
        }
    }
 out:
    return value;
}

void restructure(pq_set_t *set)
{
    node_t *pred, *cur, *h, *head;
    int i = set->max_level - 1;

    pred = head = set->head;
    while (i > 0) 
	{
		/* the order of these reads must be maintained */
        h = head->next[i]; /* record observed head */
        //CMB();
		cur = pred->next[i]; /* take one step forward from pred */
		if (!is_marked_ref(h->next[0])) 
		{
			i--;
            continue;
        }
        /* traverse level until non-marked node is found
         * pred will always have its delete flag set
         */
		while(is_marked_ref(cur->next[0])) 
		{
			pred = cur;
			cur = pred->next[i];
        }
        assert(is_marked_ref(pred->next[0]));
	
        /* swing head pointer */
        if (CAS(&head->next[i],h,cur))
            i--;
    }
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

uint32_t pq_size(pq_set_t *set) 
{
	uint32_t count=0;
	node_t* prev_node;
	node_t* item_node;
	skey_t prev_key=0, key=0;
	
	item_node = prev_node = set->head;
	while(item_node != NULL && item_node!=set->tail)
	{		
		if(!is_marked_ref(item_node->next[0]) && !is_marked_ref(prev_node->next[0]) && item_node != set->head)
		{
			count+=1;
		}
		prev_node =item_node;
		item_node = get_unmarked_ref(item_node->next[0]);
	}
    return count;
}