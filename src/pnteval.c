/**
 * @file pnteval.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#include <stdlib.h>
#include "pmproblem.h"

//Given a problem, match, and context object, this routine fills out the
//context and prepares it for use in determining pose.
int initial_context(PntMatchProblem problem, Match sol, context_handle* ch)
{
  int i,j;

  ch->pairs = 0;
  //build the initial context
  for(i = 0; i < problem->context_size; i++)
    ch->partial[i] = 0.0;
  for (i = 0; i < problem->data->size; i++)
    ch->paired[i] = 0;
  for (i = 0; i < sol->size; i++) {
    if (sol->d[i] == -1) continue;
    ch->pairs++;
    ch->paired[sol->d[i]] = 1;
    problem->context_for_pair(problem->model->x[sol->m[i]],
			      problem->model->y[sol->m[i]],
			      problem->data->x[sol->d[i]],
			      problem->data->y[sol->d[i]],
			      ch->scratch);
    for(j = 0; j < problem->context_size; j++)
      ch->partial[j] += ch->scratch[j];
  }
  return ch->pairs;
}

/* Evaulte a the fit of a solution, and return the error in the
   fit. Model should be a transformed version of the model, not the
   original.  Terminates when error exceeeds current best. */

double fitting_error(PntMatchProblem problem, Match match,
		     double best)
{
  double err = 0.0;
  int pairings = 0;
  int mp,dp,i;
  double t1,t2,tx,ty;
  double tmp;

  PointSet model;
  PointSet data;
  Pose pose;

  //grab and store model/data sets. save repeated offset from problem
  model = problem->model;
  data = problem->data;
  pose = match->pose;

  for (i = 0; i < match->size; i++) {
    // grab and store model/data pair
    dp = match->d[i];
    if (dp == -1) {
      best -= 1.0;
      continue; // skip if model point not paired
    }
    
    mp = match->m[i];
    
    //ok, lazy transform
    //we transform only points in the match
    //we break apart the trnasform, to try and help optimizers
    tx = model->x[mp];
    ty = model->y[mp];
    problem->transform(&tx,&ty,match->pose);
    
    //compute additional fitting error on this point, and to sum
    t1 = tx - data->x[dp]; t1 *= t1;
    t2 = ty - data->y[dp]; t2 *= t2;
    tmp = t1 + t2;
    err += tmp;
    best -= (tmp / problem->sigma);

    //bail if we know we've exceed our bounds
    //we return max penalty to signal that we did not do a full eval
    
    if (best < 0.0) return BAD_MATCH_PENALTY;
    pairings++; //count this pair
  }
  //don't check # pairings here to ensure valid pose, now done in pose
  //routines as it should
  return (err/problem->sigma) + ((double) (model->size - pairings));
}

//this function returns a copy of a pointset, transformed by the given pose
//its requires a pointer to the transform function, usually found as the
//transform element of a PointMatchProblem
//This function doesn't really belong here, or anywhere else, so it stays here
//for historical reasons
PointSet transform_pointset(PointSet pset, Pose pose,
			    void (*trfunc)(double*, double*, Pose))
{
  PointSet trset;
  int i;

  trset = copy_pointset(pset);  
  for (i = 0; i < trset->size; i++)
    trfunc(trset->x+i,trset->y+i,pose);

  return trset;
}

void transform_pointset_inplace(PointSet pset, Pose pose,
				void (*trfunc)(double*, double*, Pose),
				PointSet trset)
{
  int i;
  
  for (i = 0; i < trset->size; i++) {
    trset->x[i] = pset->x[i];
    trset->y[i] = pset->y[i];
    trfunc(trset->x+i,trset->y+i,pose);
  }
}

int model_pose(PntMatchProblem problem, Match match)
{
  context_handle* ch;
  
  ch = get_search_context(problem);
  initial_context(problem,match,ch);
  
  if (ch->pairs < problem->min_pairs) {
    free_search_context(NULL,ch);
    return 0;
  }
  
  if (match->pose == NULL)
    match->pose = (Pose) malloc(sizeof(double) * problem->pose_dim);
  
  problem->pose_from_partial(ch->partial,match->pose);
  free_search_context(NULL,ch);
  return 1;
}

double evaluate_match(PntMatchProblem problem, Match match, double best)
{
  int valid;

  valid = model_pose(problem,match);
  if (!valid) {
    match->error = BAD_MATCH_PENALTY;
    return match->error;
  }

  match->error = problem->degeneracy(problem->model,match->pose,
				     problem->scale);

  if (match->error > best) return match->error;
  best -= match->error;
  match->error += fitting_error(problem,match,best);
  return match->error;
}

double evaluate_match_with_partial(PntMatchProblem problem, Match match,
				   double best, double* partial)
{
  if (match->pose == NULL)
    match->pose = (Pose) malloc(sizeof(double) * problem->pose_dim);
  
  problem->pose_from_partial(partial,match->pose);
  
  match->error = problem->degeneracy(problem->model,match->pose,
				     problem->scale);

  if (match->error > best) return match->error;
  best -= match->error;
  match->error += fitting_error(problem,match,best);
  return match->error;
}

void pose_to_hetro(Pose in, double* out, int dim)
{
  int i;

  out[0] = 1.0; out[1] = 0.0; out[2] = 0.0;
  out[3] = 0.0; out[4] = 1.0; out[5] = 0.0;
  out[6] = 0.0; out[7] = 0.0; 

 switch (dim) {
  case TRANSLATION:
    out[2] = in[0];
    out[5] = in[1];
    break;
  case SIMILARITY :
    out[0] = in[0];
    out[1] = -in[1];
    out[2] = in[2];
    out[3] = in[1];
    out[4] = in[0];
    out[5] = in[3];
    break;
  case AFFINE :
    for (i = 0; i < 6; i++)
      out[i] = in[i];
    break;
  case PROJECTIVE :
    for (i = 0; i < 8; i++)
      out[i] = in[i];
    break;
 }
}

//the pose from evaluate match is incorrect with respect to the 
//original data. This corrects for that. Pose returned is always
//projective, regardless of original. This routine is needed
//because from here we usually ship the data to be warped and
//image written.
void proper_pose(PntMatchProblem problem, Match sol)
{
  context_handle* ch;
  double* np;
  int i,j;
   
  ch = get_search_context(problem);
  if (sol->pose != NULL) free(sol->pose);
  sol->pose = ch->pose;
  
  //we duplicate most of the initial context setup here, with the deviation
  //that we use the unnormalized version of the problem.

  for(i = 0; i < problem->context_size; i++)
    ch->partial[i] = 0.0;
  for (i = 0; i < problem->data->size; i++)
    ch->paired[i] = 0;
  for (i = 0; i < sol->size; i++) {
    if (sol->d[i] == -1) continue;
    ch->pairs++;
    ch->paired[sol->d[i]] = 1;
    problem->context_for_pair(problem->un_model->x[sol->m[i]],
			      problem->un_model->y[sol->m[i]],
			      problem->un_data->x[sol->d[i]],
			      problem->un_data->y[sol->d[i]],
			      ch->scratch);
    for(j = 0; j < problem->context_size; j++)
      ch->partial[j] += ch->scratch[j];
  }
  
  problem->pose_from_partial(ch->partial,sol->pose);
  
  np = (double*) malloc(sizeof(double) * 8);
  pose_to_hetro(sol->pose,np,problem->pose_dim);
  sol->pose = np;
  free_search_context(NULL,ch);
}
