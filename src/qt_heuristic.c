/**
 * @file qt_heuristic.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/


/* RANSAC search algorithm. Takes an initial start position, fills it out.
   Evalutation score is model size minus pairs, to formulate optimization
   as minimization, fitting w/ local search routines.
*/

#include <stdlib.h>
#include "pntset.h"
#include "pntmatch.h"
#include "pmproblem.h"
#include "matcheqpose.h"
#include "jadutil.h"
#include "random.h"

struct quarter_helper {
  int x;
  int y;
  int orig_pos;
};

int comp_x(const void* d1, const void* d2)
{
  struct quarter_helper* qh1;
  struct quarter_helper* qh2;

  qh1 = (struct quarter_helper*) d1;
  qh2 = (struct quarter_helper*) d2;

  if (qh1->x < qh2->x) return -1;
  else if (qh1->x > qh2->x) return 1;
  else return 0;
}

int comp_y(const void* d1, const void* d2)
{
  struct quarter_helper* qh1;
  struct quarter_helper* qh2;

  qh1 = (struct quarter_helper*) d1;
  qh2 = (struct quarter_helper*) d2;

  if (qh1->y < qh2->y) return -1;
  else if (qh1->y > qh2->y) return 1;
  else return 0;
}

// returns an array such each that element gives the quadrant that the
//corresponding element of pset is in.

char* quarter_pointset(PointSet pset)
{
  int i;
  char* qlist;
  struct quarter_helper* proxy;

  qlist = (char*) malloc(sizeof(char) * pset->size);
  proxy = (struct quarter_helper*) malloc(sizeof(struct quarter_helper) * 
					  pset->size);
  for (i = 0; i < pset->size; i++) {
    qlist[i] = 0;
    proxy[i].x = pset->x[i];
    proxy[i].y = pset->y[i];
    proxy[i].orig_pos = i;
  }

  qsort(proxy,pset->size,sizeof(struct quarter_helper),comp_x);
  for (i = pset->size/2; i < pset->size; i++)
    qlist[proxy[i].orig_pos] += 1;

  qsort(proxy,pset->size,sizeof(struct quarter_helper),comp_y);
  for (i = pset->size/2; i < pset->size; i++)
    qlist[proxy[i].orig_pos] += 2;

  free(proxy);
  return qlist;
}

// Generates a random match, such that the four initial model points are
// drawn from each of the four quadrants of the image plane.
// This is a heuristic for ransac.
Match random_quarter_match(PntMatchProblem problem, char* qlist)
{
  Match match;
  int i,j,k;

    match = allocate_match(4);
    match->size = 4;
    match->error = problem->model->size - 4.0;
    
    for (i = 0; i < 4; i++) {
      k = randint(problem->model->size-1) + 1;
      j = -1;
      do {
	j = (j + 1) % problem->model->size;
	if (qlist[j] == i) k--;
      } while (k);
      match->m[i] = j;
      match->d[i] = random() % problem->data->size;  
    }

  return match;
}


