/**
 * @file qsort_2t.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

int number_of_processors(void);

typedef struct
{
  void* base;
  size_t numel;
  size_t szof;
  int (*cmpfunc)(const void*, const void*);
} qsort_mt_data;

void* qsort_mt_thread_func(void* p)
{
  qsort_mt_data* parms;
  parms = p;
 
  qsort(parms->base,parms->numel,parms->szof,parms->cmpfunc);
  return NULL;
}

/**
 * @brief Multi-threaded replacement for qsort.
 *
 * Multi-threaded qsort routine. Sub-divides list and hands of the
 * pieces to one thread per CPU for sorting. After each thread
 * terminates the main thread merges the list (a linear operation). On
 * short lists (25 elements of less) this routine just calls qsort.
 *
 * On SMP systems with M process performance should be N/2 * log(N/2) +
 * + N + C where C is a over head for spawning M-1 threads. Note that this
 * is wall clock time, not processor time.
 *
 * This implementation is suitable as the basis for a true multi-process qsort.
 * However, the final merge sort at the end needs to be multi-threaded first.
 * The runtime return for this is minimal, and probably not worth the effort.
 *
 * @param list The list to be sorted.
 * @param numel The number of elements on the list.
 * @param szof The size of each item on the list.
 * @param cmpfunc The comparator function.
 **/

void qsort_2t(void* list, size_t numel, size_t szof,
	      int (*cmpfunc)(const void*,const void*)) {
  int np;
  int* nel;
  char** start;
  int i,ex;
  pthread_t* tids;
  qsort_mt_data* parms;
  char* buffer;
  int npos = 0;
  int base;
  char* list_as_char;
  int next_taken;
  
  np = number_of_processors();
  //do regular qsort if only one processor
   if (np == 1 || numel < 50) {
     qsort(list,numel,szof,cmpfunc);
    return;
     }
  
  np = 2; //force this to only use 2 processors,

  //determine the number of elements each thread should sort
  base = numel / np;
  ex = numel % np;
  nel = (int*) malloc(sizeof(int) * np);
  for (i = 0; i < np; i++) {
    nel[i] = base;
    if (ex) {
      nel[i]++;
      ex--;
    }
  }

  //determine start point of each sorting function
  start = (char**) malloc(sizeof(void*) * np);
  start[0] = list;
  for (i = 1; i < np; i++)
    start[i] = start[i-1] + (nel[i-1]*szof);
  
  tids = (pthread_t*) malloc(sizeof(pthread_t) * (np-1));
  parms = (qsort_mt_data*) malloc(sizeof(qsort_mt_data) * (np-1));
  for (i = 1; i < np; i++) {
    parms[i-1].base = start[i];
    parms[i-1].numel = nel[i];
    parms[i-1].szof = szof;
    parms[i-1].cmpfunc = cmpfunc;
    pthread_create(tids+(i-1),NULL,qsort_mt_thread_func,parms+(i-1));
  }
  qsort(start[0],nel[0],szof,cmpfunc);
  

  for (i = 0; i < (np-1); i++)
    pthread_join(tids[i],NULL);
  
  base = 0;
  buffer = (char*) malloc(numel * szof);

  //to get true multiprocess qsort for n threads, replace this
  //code to merge lists
  do {
    //determine the lowest top element
    next_taken = 0;
    while (!nel[next_taken]) next_taken++;
    for (i = next_taken; i < np; i++) {
      if (!nel[i]) continue; 
      if (cmpfunc(start[next_taken],start[i]) == 1)
		next_taken = i;
    }
    //copy element into new buffer
    for (i = 0; i < szof; i++)
      buffer[base+i] = start[next_taken][i];
    nel[next_taken]--;
    start[next_taken] += szof;
    npos++; 
    base += szof;
  } while (npos < numel);

  list_as_char = (char*) list;
  for (i = 0; i < numel * szof; i++)
    list_as_char[i] = buffer[i];

  free(buffer);
  free(start);
  free(nel);
  free(tids);
  free(parms);

}
