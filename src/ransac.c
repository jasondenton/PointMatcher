/**
 * @file ransac.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

/* Routines to convert between pose and matches. Much of the work for 
   ransac gets done here. The ransac file contains the actual ransac interface
   and heuristics for running ransac. The logic is that routines in here
   might be used for other purposes in other search algorithsm. */

#include <stdlib.h>
#include <math.h>

#include "pntset.h"
#include "pntmatch.h"
#include "pmproblem.h"
#include "jadutil.h"
#include "lsearch.h"

// This routine takes two point sets, and returns a match with represents 
// the correspondence between the two given no further transformation.
// The routine finds the distance between all pairs of points, and then
// adds pairs to the match in the order of least distance. The match is
// constrained to be one to one. This routine sits at the heart of ransac.
// The ransac routine is really just a wrapper around the book
// keeping needed to use this routine in that context.

//the match member passed must have sufficent space for the pairs to be
//placed in it, usually this means min(ms,ds). 

void* init_ransac_context(void* p)
{
  RansacContext* rc;
  PntMatchProblem problem;
  int i;
  
  problem = (PntMatchProblem) p;
  rc = (RansacContext*) malloc(sizeof(RansacContext));
  rc->dtaken = malloc_array(char,problem->data->size);
  rc-> mtaken = malloc_array(char,problem->model->size);
  rc->dist_table = malloc_array(double*,problem->model->size);
  for (i = 0; i < problem->model->size; i++) 
    rc->dist_table[i] = malloc_array(double,problem->data->size);
  rc->trset = allocate_pointset(problem->model->size);
  rc->ch = get_search_context(problem);
  return (void*)rc;
}

void free_ransac_context(void* foo, void* r)
{
  RansacContext* rc;
  
  rc = (RansacContext*) r;
  free(rc->mtaken);
  free(rc->dtaken);
  free_list((void**)rc->dist_table,rc->trset->size);
  free_pointset(rc->trset);
  free_search_context(NULL,rc->ch);
  free(rc);
}

Match closest_match_pairs(RansacContext* rc, PointSet data, double sigma,
			  Match match)
{
  int m,d;
  int i,j;
  double tmp;
  int pair_num = 0;
  
  //init section
  m = rc->trset->size;
  d = data->size;

  match->error = rc->trset->size;
  
  for (i = 0; i < d; i++)
    rc->dtaken[i] = 0;
  
  //this pair of loops builds the distance table
  for (i = 0; i < m; i++) {
    rc->mtaken[i] = 0;
    //dist_table[i] = (double*) malloc(sizeof(double) * d);
    for (j = 0; j < d; j++) {
      tmp = (rc->trset->x[i] - data->x[j]);
      tmp *= tmp;
      rc->dist_table[i][j] = tmp;
      tmp = (rc->trset->y[i] - data->y[j]);
      tmp *= tmp;	    
      rc->dist_table[i][j] += tmp;
    }
  }
  
  tmp = 0.0;
  while (tmp <= sigma) {
    tmp = 999999999.9999999;
    m = -1;  //reuse these
    d = -1;
    //scan the table. m,d gets the closest pair
    for (i = 0; i < rc->trset->size; i++)
      for (j = 0; j < data->size; j++) 
	if (rc->dist_table[i][j] < tmp) {
	  tmp = rc->dist_table[i][j];
	  m = i;
	  d = j;
	}
    //if closest pair is less than sigma, add it to the match
    if (tmp <= sigma) {
      //make sure we don't pull this entry out again
      rc->dist_table[m][d] = sigma * 512.0;
      if (rc->mtaken[m]) continue;  rc->mtaken[m] = 1;
      if (rc->dtaken[d]) continue;  rc->dtaken[d] = 1;
      match->m[pair_num] = m;
      match->d[pair_num] = d;
      pair_num++;
      match->error -= 1.0;  
    }
  }
  match->size = pair_num;
  
  return match;
}

//to use fast ransac :
//probe and result must have pose already allocated
//rc must have been allocated
int ransac_actual(PntMatchProblem problem, RansacContext* rc, 
		 Match probe, Match result)
{
  initial_context(problem,probe,rc->ch);
  if (rc->ch->pairs < problem->min_pairs) return 0;
  problem->pose_from_partial(rc->ch->partial,probe->pose);
  transform_pointset_inplace(problem->model,probe->pose,
			     problem->transform,rc->trset);
  closest_match_pairs(rc,problem->data, problem->sigma,result);
  return 1;
}

int iransac_actual(PntMatchProblem problem, RansacContext* rc, 
		 Match prev, Match match)
{
  Match best;
  int steps = 0;
  int ittr = 0;
  Pose pose;

  pose = (Pose) malloc(sizeof(double) * problem->pose_dim);
  best = copy_match(prev);
  do {
    best->pose = pose;
    ransac_actual(problem,rc,best,match);
    steps++;
    if (match->size < best->size) {
      replace_match(match,best);
      return steps;
    }
    else if (match->size == best->size) ittr++;
    else ittr = 0;
    best->pose = NULL;
    free_match(best);
    best = copy_match(match);
  } while (ittr < 3);
  free(pose);
  return steps;
}

// Do one iteration of ransac. Given an initial match, transform the
// model appropriately and then find all corresponding points. Return
// the resulting match. This version matches the local search routine
int ransac(PntMatchProblem problem, Match probe)
{
  RansacContext* rc;

  rc = init_ransac_context(problem);
  ransac_actual(problem,rc,probe,probe);
  free_ransac_context(NULL,rc);

  return 1;
}

// Iterate on ransac till no futher improvements found. this version matches
// the local search routine.
int iransac(PntMatchProblem problem, Match probe)
{
  
  RansacContext* rc;
  int steps = 0;

  rc = init_ransac_context(problem);
  while (ransac_actual(problem,rc,probe,probe)) steps++;
  free_ransac_context(NULL,rc);

  return !(steps > 0);
}

unsigned long expected_ransac_trials(PntMatchProblem problem, double odds)
{
  double tmp;
  double pairs;
  double ms, ds;

  if (problem->solution) pairs = problem->solution->size;
  else pairs = problem->model->size * 0.75;

  ms = problem->model->size;
  ds = problem->data->size;
  //compute m^4d^4 portion
  tmp = ms * ds; 
 ms--; ds--;
  tmp *= ms * ds;
  ms--; ds--;
  tmp *= ms * ds;
  ms--; ds--;
  tmp *= ms * ds;
  pairs *= (pairs - 1.0) * (pairs - 2.0) * (pairs - 3.0);
  tmp = log(1.0-odds)/log(-(pairs - tmp)/tmp);
  tmp += 1.0;
  return (unsigned long) tmp;
}
