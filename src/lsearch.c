/**
 * @file lsearch.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdio.h>
#include <stdlib.h>

#include "pmproblem.h"

/*
 * Take one step of local search, in a steepest descent manner. 
 * Neighborhood is all matches that differ from the initial match
 * one additional pair, one removed pairing, or one model point paired to a 
 * different data point.
 *
 * problem : The problem descriptor.
 * sol ; The initial match from which to search. Should be in the "expanded"
 *       format. Condesnsed matches do not work. As part of this, pose
 *       pose should be allocated.
 * prev : The index of the pairing changed in the previous step, or -1 if none.
 * context : The block containing the current pre-computed context for step
 *           step by step search.  Part of the problem/process_list magic.
 * pairs : 
 *
 * return : -1 means no improvement found.
 *           Otherwise returns the index of the improved pair 
 *          (for use in prev in the next round).
 */

int local_search_step(PntMatchProblem problem, Match sol, int prev, 
		      context_handle* ch)
{
  double bestvalue, curvalue;
  int data_size,i,j;
  int orig_dp;
  int best_dp = -1;
  int found = -1;
  
  //min defreferncing vars
  double* modelx;
  double* modely;
  double* datax;
  double* datay;
  double* partial;
  double* save;
  double* scratch;
  short* sold;
  char* paired;
  int pairs;
  
  modelx = problem->model->x;
  modely = problem->model->y;
  datax = problem->data->x;
  datay = problem->data->y;
  sold = sol->d;
  data_size = problem->data->size;
  
  partial = ch->partial;
  save = ch->save;
  scratch = ch->scratch;
  paired = ch->paired;
  pairs = ch->pairs;
  
  bestvalue = sol->error;
  
  //save the original context
  for (i = 0; i < problem->context_size; i++)
    save[i] = partial[i];
  
  //for each possible pair in the solution
  for (i = 0; i < sol->size; i++) {
    if (i == prev) continue;
    orig_dp = sold[i];
    
    //Get the context for this pairing, remove it from the original
    if (orig_dp > -1) {
      problem->context_for_pair(modelx[i],modely[i],
				datax[orig_dp],datay[orig_dp],scratch);
      for (j = 0; j < problem->context_size; j++)
	partial[j] -= scratch[j];
    }
    
    //Next, check see if we are removing a pair as the first step
    if (orig_dp != -1 && (pairs-1) >= problem->min_pairs) {
      sold[i] = -1;
      curvalue=evaluate_match_with_partial(problem,sol,bestvalue,partial);
      if (curvalue < bestvalue) {
	best_dp = -1;
	bestvalue = curvalue;
	found = i;
      }
    }
    
      //for each possible different match
      for (sold[i] = 0; sold[i] < data_size; sold[i]++) {
	if (paired[sold[i]]) continue;
	//get context for pair, add it in
	problem->context_for_pair(modelx[i],modely[i],
				  datax[sold[i]],datay[sold[i]],scratch);
	for (j = 0; j < problem->context_size; j++)
	  partial[j] += scratch[j];
	curvalue=evaluate_match_with_partial(problem,sol,bestvalue,partial);
  //curvalue=evaluate_match_with_partial(problem,sol,9999999999.99,partial);
	if (curvalue < bestvalue) {
	  best_dp = sold[i];
	  bestvalue = curvalue;
	  found = i;
	}
	//reset the context for the next pair 
	for (j = 0; j < problem->context_size; j++)
	  partial[j] -= scratch[j];
      } // end of different pair for each m[i] loop

      sold[i] = orig_dp;
      for (j = 0; j < problem->context_size; j++)
	partial[j] = save[j];

    } //end of per pair loop
  
    sol->error = bestvalue;
    if (found != -1) { //if we found an improvement
      //do we need to remove a context?
      if (best_dp == -1) { //drop because whole pair going away
	paired[sold[found]] = 0;
	problem->context_for_pair(modelx[found], modely[found],
				  datax[sold[found]], datay[sold[found]],
				  scratch);
	for(j = 0; j < problem->context_size; j++)
	  partial[j] -= scratch[j];
      }
      //we need to add in a pair since best_dp != -1
      else {
	if (sold[found] != -1) {
	  problem->context_for_pair(modelx[found], modely[found],
				    datax[sold[found]], datay[sold[found]],
				    scratch);
	  for(j = 0; j < problem->context_size; j++)
	    partial[j] -= scratch[j];
	  paired[sold[found]] = 0;
	}

	paired[best_dp] = 1;
	problem->context_for_pair(modelx[found], modely[found],
				  datax[best_dp], datay[best_dp], scratch);
	for(j = 0; j < problem->context_size; j++)
	  partial[j] += scratch[j];
      }
      
      sold[found] = best_dp;
      return found;
    }
    else return -1;
}


int local_search_quick_step(PntMatchProblem problem, Match sol, int prev, 
		      context_handle* ch)
{
    double bestvalue, curvalue;
    double tx,ty;
    double tmp,qerr;
    double maxdist;
    int data_size,i,j;
    int orig_dp;
    int best_dp = -1;
    int found = -1;
  
    //min defreferncing vars
    double* modelx;
    double* modely;
    double* datax;
    double* datay;
    double* partial;
    double* save;
    double* scratch;
    short* sold;
    char* paired;
    int pairs;
    
    modelx = problem->model->x;
    modely = problem->model->y;
    datax = problem->data->x;
    datay = problem->data->y;
    sold = sol->d;
    data_size = problem->data->size;
    maxdist = problem->sigma * 2.0;
    
    partial = ch->partial;
    save = ch->save;
    scratch = ch->scratch;
    paired = ch->paired;
    pairs = ch->pairs;
    
    bestvalue = sol->error;
    problem->pose_from_partial(partial,ch->extra_pose);
    
    //save the original context
    for (i = 0; i < problem->context_size; i++)
      save[i] = partial[i];
    
    //for each possible pair in the solution
    for (i = 0; i < sol->size; i++) {		
      if (i == prev) continue;
      orig_dp = sold[i];
      
      tx = modelx[i];
      ty = modely[i];
      problem->transform(&tx,&ty,ch->extra_pose);
      
      //Get the context for this pairing, remove it from the original
      if (orig_dp > -1) {
	problem->context_for_pair(modelx[i],modely[i],
				  datax[orig_dp],datay[orig_dp],scratch);
	for (j = 0; j < problem->context_size; j++)
	  partial[j] -= scratch[j];
      }
      
      //Next, check to see if we are removing a pair as the first step
      if (orig_dp != -1) { //removed min pairs check cause qstep is immune
	sold[i] = -1;
	curvalue=evaluate_match_with_partial(problem,sol,bestvalue,partial);
	if (curvalue < bestvalue) {
	  best_dp = -1;
	  bestvalue = curvalue;
	  found = i;
	}
      }
      
      //for each possible different match
      for (sold[i] = 0; sold[i] < data_size; sold[i]++) {
	if (paired[sold[i]]) continue;
	
	//place in qerr the squared distance between model/data
	qerr = tx - datax[sold[i]];
	qerr *= qerr;
	tmp = ty - datay[sold[i]];
	tmp *= tmp;
	qerr += tmp;
	
	if (qerr > maxdist) continue;
	
	//get context for pair, add it in
	problem->context_for_pair(modelx[i],modely[i],
				  datax[sold[i]],datay[sold[i]],scratch);
	for (j = 0; j < problem->context_size; j++)
	  partial[j] += scratch[j];
	curvalue=evaluate_match_with_partial(problem,sol,bestvalue,partial);
	if (curvalue < bestvalue) {
	  best_dp = sold[i];
	  bestvalue = curvalue;
	  found = i;
	}
	//reset the context for the next pair G
	for (j = 0; j < problem->context_size; j++)
	  partial[j] -= scratch[j];
      } // end of different pair for each m[i] loop
      
      sold[i] = orig_dp;
      for (j = 0; j < problem->context_size; j++)
	partial[j] = save[j];
      
    } //end of per pair loop
    
    sol->error = bestvalue;
    if (found != -1) { //if we found an improvement
      //do we need to remove a context?
      if (best_dp == -1) { //drop because whole pair going away
	paired[sold[found]] = 0;
	problem->context_for_pair(modelx[found], modely[found],
				  datax[sold[found]], datay[sold[found]],
				  scratch);
	for(j = 0; j < problem->context_size; j++)
	  partial[j] -= scratch[j];
      }
      //we need to add in a pair since best_dp != -1
      else {
	if (sold[found] != -1) {
	  problem->context_for_pair(modelx[found], modely[found],
				    datax[sold[found]], datay[sold[found]],
				    scratch);
	  for(j = 0; j < problem->context_size; j++)
	    partial[j] -= scratch[j];
	  paired[sold[found]] = 0;
	}
	
	paired[best_dp] = 1;
	problem->context_for_pair(modelx[found], modely[found],
				  datax[best_dp], datay[best_dp], scratch);
	for(j = 0; j < problem->context_size; j++)
	  partial[j] += scratch[j];
      }
      
      sold[found] = best_dp;
      return found;
    }
    else return -1;
}

//context is a per thread scratch space. Find it as :
//for each thread of execution we need :
//2) one "context" to hold the current intermediate pose calculations
//3) one context to hold the intermediate calculations on the last step
//actually taken, this is the position we revert to when trying a
//different pair.
//4) one context to hold the intermediate calculations for just this pair.
//The context_for_pair routine places results here, and pose
//calcuation combines 4 & 5 to get 3.
//5) an array of size data points, which tells us quickly if a data point
//is available for matching.


int local_search(PntMatchProblem problem, Match sol, context_handle* ch)
{
  int pstep = -1;
  int steps = 0;
  int threshhold;
  
  threshhold = problem->min_pairs * 2;
  
  if (sol->size != problem->model->size)
    expand_match(sol,problem->model->size);
  
  initial_context(problem,sol,ch);
  
  //keep stepping as long as we can
  pstep = local_search_step(problem,sol,pstep,ch);
  while (pstep != -1) {
    if (sol->d[pstep] != -1) ch->pairs++;
    else ch->pairs--; 
    steps++;
    if (ch->pairs >= threshhold)
      pstep = local_search_quick_step(problem,sol,pstep,ch); 
    else
      pstep = local_search_step(problem,sol,pstep,ch);
  }
  return steps;
}
