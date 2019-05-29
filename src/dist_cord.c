/**
 * @file dist_cord.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made available
 * under the terms of the new BSD license. See the file license.txt
 * for the applicable terms.

   The process_list function provides a high level way to apply a
   given function to every element on a list and keep track of the
   resulting return values. On single processor systems this is done
   with a simple for loop. On SMP systems the work is farmed out to
   all available CPUs. process_list attempts to keep all CPUs working
   on the problem until all items on the list have been processed, and
   to do so in such a way as to minimize the number of times a mutex
   must be locked and unlocked. It does this by magic.

   process_list is called with a list_proc_obj, which describes the
   list to be processed and the function which should be called for
   each item on the list. Additionally, there are provisions for
   passing both a global parameter block to each thread and for
   creating and managing a per-thread context object.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#include "jadmulti.h"
#include "jadutil.h"
#include "jadmath.h"

//Data structure used to track the list to be processed, the threads
//processing it, and the helper functions used to process that list.

typedef struct {
  void** list_data;
  void** retlist;
  unsigned long list_size;
  int* next;
  int* last;
  int* inc;
  int cpus;
  int reserve;
  int finished;
  int context_size;
  void** context;
  pthread_t* tids;
  pthread_mutex_t* mutex;
  void* pdata;
  void* (*action_func)(void*,void*,void*);
  void (*free_scratch_space)(void*,void*);
} Coordinator_Data;

typedef Coordinator_Data* Coordinator;

//Helper function to setup a new coordinator
Coordinator new_coordinator(list_proc_obj lpo,int cpus)			    
{
  Coordinator coord; 
  int i,j;
  
  coord = (Coordinator) malloc(sizeof(Coordinator_Data));
  if (!coord) return NULL;
  
  coord->list_data = lpo.list;
  coord->list_size = lpo.list_size;
  coord->next = (int*) malloc(sizeof(int) * cpus);
  coord->last = (int*) malloc(sizeof(int) * cpus);
  coord->inc = (int*) malloc(sizeof(int) * cpus);
  coord->retlist = (void*) malloc(sizeof(void*) * lpo.list_size);
  coord->cpus = cpus;
  coord->tids = (pthread_t*) malloc(sizeof(pthread_t) * cpus);
  coord->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(coord->mutex,NULL);
  coord->action_func = lpo.process_item;
  coord->pdata = lpo.shared;
  coord->finished = 0;
  coord->context = (void**) malloc(sizeof(void*) * cpus);
  //not optimal for 1 processors, but the front end interface
  //should screen for that case, and never do this for 1 processor
  j = ((int)(lpo.list_size * 0.75))/cpus; //init list size per cpu
  for (i = 0; i < cpus; i++) {
    coord->next[i] = i;
    coord->last[i] = cpus*j+i;
    coord->inc[i] = cpus;
    if (lpo.allocate_scratch_space) 
      coord->context[i] = lpo.allocate_scratch_space(lpo.shared);
    else coord->context[i] = NULL;
  }
  coord->free_scratch_space = lpo.free_scratch_space;
  coord->reserve = coord->last[cpus-1]+1;
  return coord;
}

//Helper function to cleanup a coordinator
void delete_coordinator(Coordinator coord)
{
  int i;
  for (i = 0; i < coord->cpus; i++)
    if (coord->context[i] != NULL) {
      if (coord->free_scratch_space)
	coord->free_scratch_space(coord->pdata,coord->context[i]);
      else free(coord->context[i]);
    }

  free(coord->context);  
  free(coord->next);
  free(coord->last);
  free(coord->inc);
  free(coord->tids);
  pthread_mutex_destroy(coord->mutex);
  free(coord->mutex);
  free(coord);
}

//Return the next item that should be processed by this thread.
//It is this function which is responsible for implementing the load
//balancing algorithm, after the initial assignment.
unsigned long next_item(Coordinator coord, int id) {
  
  int next_item;
  int take;
  
  next_item = coord->next[id];
  
  if (next_item > coord->last[id]) {
    pthread_mutex_lock(coord->mutex);
    if (coord->reserve >= coord->list_size) {
      pthread_mutex_unlock(coord->mutex);
      return -1;
    }
    next_item = coord->reserve;
    if (coord->finished < coord->cpus/2) 
      take = (coord->list_size - coord->reserve) /
	(coord->cpus - coord->finished);
    else 
      take = (coord->list_size - coord->reserve) / (coord->cpus/2);
    take = take < 5 ? 5 : take; //no alloc less than 5 elements
    coord->reserve += take;
    coord->finished++;
    pthread_mutex_unlock(coord->mutex);
    coord->last[id] = next_item + take - 1;
    coord->last[id] = min(coord->last[id],coord->list_size-1);
    coord->next[id] = next_item;
    coord->inc[id] = 1;
  }
  
  coord->next[id] += coord->inc[id];
  
  return next_item;
}


//This is the entry point for each thread.
void* dc_thread_action(void* ctmp)
{
  Coordinator coord;
  int myid = -1;
  pthread_t mytid;
  unsigned long idx;
 
  coord = (Coordinator) ctmp;
  mytid = pthread_self();
  //figure out which one is calling this function
    for (idx = 0; idx < coord->cpus; idx++) {
      if (coord->tids[idx] == mytid) myid = idx;
    }
  //loop unil next_item tells us we are done.
    while((idx = next_item(coord,myid)) != -1) 
    coord->retlist[idx] =
      coord->action_func(coord->pdata,coord->context[myid],
			 coord->list_data[idx]);
  return NULL;
}

//This function processes the list if multiple processors are available.
void** process_list_mt(Coordinator coord)
{
  int i;
  void* toss;
  void** ret;
  
  coord->tids[0] = pthread_self(); 
  for (i = 1; i < coord->cpus; i++) 
    pthread_create(coord->tids+i,NULL,dc_thread_action,coord);
  
  dc_thread_action(coord); //finally, main thread does it stuff

  //wait for others to join
   for (i = 1; i < coord->cpus; i++) 
     pthread_join(coord->tids[i],&toss);

  ret = coord->retlist;
  return ret;
}

//This function processes the list if there is only one processor.
void** process_list_st(list_proc_obj lpo)
{ 
  unsigned long i;
  void* scratch = NULL;
  void** retlist;
  
  retlist = malloc_array(void*,lpo.list_size);
  
  if (lpo.allocate_scratch_space)
    scratch = lpo.allocate_scratch_space(lpo.shared);
  for (i = 0; i < lpo.list_size; i++)
    retlist[i] = lpo.process_item(lpo.shared,scratch,lpo.list[i]);
  if (scratch) {
    if (lpo.free_scratch_space) lpo.free_scratch_space(lpo.shared,scratch);
    else free(scratch);
  }
  return retlist;
}

/**
 * @brief Process a list using multiple processors, if available.
 
 * The function process_list calls a specified function once for each
 * object appareing on a specified list. The results of calling this
 * function are placed in the returned array, in a position
 * corresponding to the placement of the original object. For each
 * processor available beyond the first an additional thread is
 * created and the work is shared between all threads. When all
 * objects on the given list have been processed, any additional
 * threads are cleaned up. No guareentess are made about the order in
 * which items on the list will be processed, only that they will have
 * all been processed when this function returns.
 *
 * This function attempts to be smart about how items are handed out
 * to threads for processing, so that each thread keeps working until
 * the entire list is processed, while minimizing the number of mutex
 * locks that must take place. The current heuristic for achieving
 * this assumes that items which appear close to each other on the
 * list to be processed will require similiar amounts of computation
 * to process. For instance, perhaps the provided list has been sorted
 * so that items with longer required computation times appear at the
 * front of the list, with the expected computation time dropping as
 * you move down the list.
 *
 * @param lpo A list_proc_obj describing the list to be processed and
 * how to process it.
 * @return The list of objects produced by calling the process_item
 * function given in lpo on every item on the list specified by lpo.
 **/

void**  process_list(list_proc_obj lpo) {
  Coordinator coord; int num_proc;
  void** ret;
  
  num_proc = number_of_processors();

  if (num_proc == 1) ret = process_list_st(lpo);
  else {
    coord =  new_coordinator(lpo,num_proc);
    ret = process_list_mt(coord);
    delete_coordinator(coord);
  }
  return ret;
}

/**
 * @brief Create a new list processor object.
 *
 * This routine creates and initilizes a new list process object. It
 * is a convieance routine.
 *
 * @param list The list of objects to be processed.
 * @param size The number of objects on the list.
 * @param shared A block of static data, to be provided to
 * the processing routine.
 * @param function The function used to process each item on the
 * list. See item_processor for a complete description.
 * @return The newly created list processing object.
 **/

list_proc_obj get_list_proc_obj(void** list, unsigned long size,void* shared,	
				void* (*function)(void*,void*,void*))
{
  list_proc_obj lpo;
  
  lpo.list = list;
  lpo.list_size = size;
  lpo.shared = shared;
  lpo.process_item = function;
  lpo.allocate_scratch_space = NULL;
  lpo.free_scratch_space = NULL;
  return lpo;
}
