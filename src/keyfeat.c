/**
 * @file keyfeat.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#include <stdio.h>
#include <stdlib.h>
#include "pmproblem.h"
#include "jadutil.h"
#include "jadmulti.h"
#include "jadmath.h"
#include "lsearch.h"

//return list of lists, containing nearest neighbors for each point
//each elements starts with the index of the point the others are nearest too
//thus list[i][0] = i in all cases, list[i][1] is the closest point, list[i][2]
//is second closest, and so on.
short** pointset_neighbors(PointSet points, int num)
{
  short** list;
  //short** ulist;
  double* distance;
  int i,j,k;
  int size,best;
  double low,high;

  size = points->size;
  list = (short**) malloc(sizeof(short*) * size);
  for (i = 0; i < size; i++)
    list[i] = (short*) malloc(sizeof(short) * num);

  distance = (double*) malloc(sizeof(double) * size * size);

  //build table of distances
  for (i = 0; i < size; i++) {
    distance[i * size + i] = 9e20;
    for (j = 0; j < i; j++) {
      low = points->x[i] - points->x[j];
      distance[i * size + j] = low * low;
      low = points->y[i] - points->y[j];
      distance[i * size + j] += low * low;
      distance[j * size + i] = distance[i * size + j];
    }
  }

  // for each point in the set
  for (i = 0; i < size; i++) {
    list[i][0] = i;
    low = -8e20;
    best = -1; //not strictly neccessary, surpresses warnings
   // for each nearest point
    for (j = 1; j < num; j++) {
     high = 8e20;
      // for each point in the set, again
      for (k = 0; k < size; k++) {
	if (i == k) continue;
	if (distance[i*size+k] < high && distance[i*size+k] > low) {
	  best = k;
	  high = distance[i*size+k];
	}
      } //end k
      list[i][j] = best;
      low = high;
    } //end j
  } //end i

  free(distance);
  return list;
}

//permutates clusters, holding the key point (clusters[i][0]) the same
short** cluster_permutations(short** clusters, int numc, int csize,
			     int* nperms)
{
  short** newlist;
  int* plist;
  int** pmap;
  int numperm;
  int i,j,k;
  int listpos = 0;

  plist = (int*) malloc(sizeof(int) * (csize - 1));
  for (i = 0; i < csize-1; i++) plist[i] = i;
  pmap = permutations(plist,csize-1,&numperm);
  free(plist);
  newlist = (short**) malloc(sizeof(short*) * numperm * numc);
  for(i = 0; i < numc; i++) {
    for (j = 0; j < numperm; j++) {
      newlist[listpos] = (short*) malloc(sizeof(short) * csize);
      newlist[listpos][0] = clusters[i][0];
      for (k = 0; k < csize-1; k++) 
	newlist[listpos][k+1] = clusters[i][pmap[j][k]+1];
      listpos++;
    }
    free(clusters[i]);
  }

  free(clusters);
  free_ntlist((void**)pmap);

  if (nperms) *nperms = numperm * numc;
  return newlist;
}

//remove dups from the match list. We try to be clever here. 
//removing the earliest instance of a duplicate makes the algorithm
//significantly easier to write.
Match* prune_match_list(Match* features, unsigned long* numf)
{
  unsigned long unique;
  unsigned long it1, it2;
  Match* ufeat;

  unique = *numf;
  for (it1 = 0, it2 = 1; it2 < *numf; it1++,it2++)
    if (!compare_match(features+it1,features+it2)) {
      free_match(features[it1]);
      features[it1] = NULL;
      unique--;
    }
  
  ufeat = malloc_array(Match,unique);
  it2 = 0;
  for (it1 = 0; it1 < *numf; it1++) {
    if (features[it1] == NULL) continue;
    ufeat[it2] = features[it1];
    it2++;
  }
  free(features);
  *numf = unique;
  return ufeat;
}

void* eval_list_wrapper(void* prb, void* scratch, void* item)
{
  context_handle* ch;
  PntMatchProblem problem;
  Match match;

  ch = (context_handle*) scratch;
  problem = (PntMatchProblem) prb;
  match = (Match) item;
  match->pose = ch->pose;
  initial_context(problem,match,ch);
  evaluate_match_with_partial(problem,match,10000.0,ch->partial);
  match->pose = NULL;

  if (match->error > 10000.0) {
    free_match(match);
    match = NULL;
  }
  return match;
}

/**
 * key_features returns a list of partial matches, consisting of
 * non-degenerate key features.
 *
 * problem : The problem descriptor.
 * pairs : The number of pairs in each key feature (partial feature)
 * want : The number of features we want returned to from the function.
 *        A value of 0 results in all features being returned, otherwise
 *        the list is pruned so that no more than want is returned.
 *        A value of -1 results in the algorithm picking a good list.
 *        Currently, this is the top half of the list.
 * got : The number of features actually returned. May be less than want if
 *       there are not enough non-degenerate features.
 *
 * return : A list of matches, corresponding to key features. Each
 *          match has been evaluated and the error field is
 *          correct. The list is sorted in order of ascending erros
 *          score, and the trial_num field is set to reflect the
 *          matches position on the list. This facilitates later steps
 *          where features will be fully searched and resorted.  Pose
 *          information is NOT retained, pose is set to null for each
 *          match.  This is a hack to save memory. It does not affect
 *          performance in local search. It is a known performane hit
 *          for clustering.  But this algorithm eats up memory, and
 *          trade offs must be made.
 *
 * NOTE : All key features are generated and sorted, regaurdless of
 * the value of want. Prunning the list is strictly a memory reduction
 * trick, and does not effect performance (at least at this stage).
 */

Match* key_features(PntMatchProblem problem, int pairs, long want,
		    unsigned long* got)
{
  list_proc_obj lpo;
  short** model_cluster;
  short** data_cluster;
  int mc, dc;
  Match* features;
  Match* flist;
  Match curf;
  int i,j;
  int tf;

  //find nearest neighbors for both madel and data
  model_cluster = pointset_neighbors(problem->model,pairs);
  mc = problem->model->size;
  data_cluster = pointset_neighbors(problem->data,pairs);
  dc = problem->data->size;

  //find all permutations for the set with the least clusters
  if (mc < dc)
    model_cluster = cluster_permutations(model_cluster,mc,pairs,&mc);
  else
    data_cluster = cluster_permutations(data_cluster,dc,pairs,&dc);

  //allocate space for key features
  tf = mc * dc;
  features = (Match*) malloc(sizeof(Match) * tf);
  for (i = 0; i < tf; i++)
    features[i] = (Match) malloc(sizeof(MatchData));

  //construct the key features
  for (i = 0; i < mc; i++)
    for (j = 0; j < dc; j++) {
      curf = features[i*dc+j];
      curf->m = model_cluster[i];
      curf->d = data_cluster[j];
      curf->size = pairs;
      curf->allocated = 0;
      curf->pose = NULL;
    }
  //evaluate all features, using multiple processors if possible
  lpo = get_list_proc_obj((void**)features,tf,
			  (void*) problem,eval_list_wrapper);
  lpo.allocate_scratch_space = get_search_context;
  lpo.free_scratch_space = free_search_context;
  flist = (Match*) process_list(lpo);
  free(features); features = flist;
  //sort the list
  qsort_2t(features,tf,sizeof(Match),compare_match);

  //figure out where the cut off from prunning degen mathes is
  i = tf/2;
  while (i < tf && features[i]) i += max(((tf - i) / 2),1);
  if (i != tf) { 
    while(i >= 0 && !features[i]) i -= 5;
    while (features[i]) i++;
  }
  *got = i;
 
  //if want == -1 set heuristic number of trials
  //current heuristic is the top half of the list
  if (want == -1) want = min(*got,tf * 0.5);
  else if (want == 0) want = *got;
  else if (want > *got) want = *got;
  flist = (Match*) malloc(sizeof(Match) * want);

  //we expand each match, because expanding causes the match to
  //get its own copy of the pairings; prior to this we are simply
  //pointing to the clusters from earlier, and many matches share
  //there clusters.
  for (i = 0; i < want; i++) {
    expand_match(features[i],problem->model->size);
    flist[i] = features[i];
    flist[i]->trial_num = i;
  }

  for (i = want; i < *got; i++) 
    free_match(features[i]);
    
  free_list((void**)model_cluster,mc);
  free_list((void**)data_cluster,dc);
  free(features);

  *got = want;

  //printf("end of kf\n");
  return flist;
}

void* onestep_wrapper(void* prb, void* scratch, void* item)
{
  context_handle* ch;
  PntMatchProblem problem;
  Match match;
  int improve;

  problem = (PntMatchProblem) prb;
  match = (Match) item;
  ch = (context_handle*) scratch;
  match->pose = ch->pose;
  initial_context(problem,match,ch);
  improve = local_search_step(problem,match,-1,ch);
  match->pose = NULL;

  if (improve == -1 || match->error > 10000.0) {
    free_match(match);
    match = NULL;
  }
  return match;
}


// Does the same thing as key_features, and calls that funtion as a first step.
// It retrieves the entire list of features, then subjects each to one step
// of local search. It sorts and returns the resulting matches.
Match* improved_key_features(PntMatchProblem problem, int pairs, long want,
			     unsigned long* got)
{
  Match* tlist;
  Match* flist;
  int i;
  list_proc_obj lpo;

  //generate key features, return all non-degenerate features
  tlist = key_features(problem,pairs,0,got);
  printf("Got %lu non-degen features.\n",*got);
  //pruning must be done after expand, so kf does not do it by itself
  //because it frequently doesn't prune anything. but... with such a big
  //chunk of the list, we could get lots of savings if we save a few 
  //steps of ls.
  tlist = prune_match_list(tlist,got);
  printf("Got %lu unique features.\n",*got);
  
  //allow each key feature to undergo a single step of local search
  lpo = get_list_proc_obj((void**)tlist,*got,(void*) problem,onestep_wrapper);
  lpo.allocate_scratch_space = get_search_context;
  lpo.free_scratch_space = free_search_context;
  flist = (Match*) process_list(lpo);
  //process_list((void**)tlist,*got,(void*)problem,problem->context_alloc,
  //	       onestep_wrapper);
  //sort em
  qsort_2t(flist,*got,sizeof(Match),compare_match);
  
  //adjust got according to match what we have after
  //null entries are pruned. Remember, we prune
  //an entry if it is still "degenerate" after one pass
  //of local search
  i = *got/2;
  while (i < *got && flist[i]) i += max(((*got - i) / 2) ,1);
  if (i != *got) while(i >= 0 && !flist[i]) i -= 5;
  while (i < *got && flist[i]) i++;
  *got = i;
  flist = prune_match_list(flist,got);
  printf("Got %lu unique features after one-step.\n",*got);
  
   //current heuristic is the top half of the list
  if (want == -1) want = *got * 0.5; 
  else if (want == 0 || want > *got) want = *got;
 
  //hack to save a piddly amonut of memory
  tlist = (Match*) malloc(sizeof(Match) * want);
  for (i = 0; i < want; i++) {
    tlist[i] = flist[i];
    tlist[i]->trial_num = i;
  } 

  for (i = want; i < *got; i++)
    free_match(flist[i]);
  free(flist);
  *got = want;
  return tlist;
}
