/**
 * @file pntmatcher.c
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
#include <string.h>
#include <time.h>
#include "pmproblem.h"
#include "lsearch.h"
#include "qt_heuristic.h"
#include "expr_sup.h"
#include "jadutil.h"
#include "jadmulti.h"
#include "jadmath.h"
#include "random.h"
#include "manual.h"

/* These wrapper functions adapt individual search routines to the to
   the format needed by process_list. Mostly, they reuse a previosuly
   allocated scratch space for storing individual pose and then
   cleanup afterward. 
*/

/* Notice the use of compact match. This is a costly (time wise)
   operation. Worse, it hits malloc which is a multi-thread bottle
   neck. The idea is that with large searches memory needs to be
   saved. But note that in some cases all needed memory for the match
   list will have already been obtained. */

void* ls_wrapper(void* problem, void* scratch, void* item)
{
  ((Match)item)->pose = ((context_handle*)scratch)->pose;
  local_search((PntMatchProblem)problem,(Match)item,(context_handle*)scratch);
  ((Match)item)->pose = NULL;
  //compact_match(item);
  return item;
}

void* ransac_wrapper(void* extra, void* context, void* item)
{
  Match result;

  ((Match)item)->pose = ((RansacContext*)context)->ch->pose;
  result = allocate_match(((PntMatchProblem)extra)->model->size);
  ransac_actual(extra,context,item,result);
  ((Match)item)->pose = NULL;
  result->trial_num = ((Match)item)->trial_num;
  free_match((Match)item);
  compact_match(result);
  sort_match(result);
  return result;
}

void* iransac_wrapper(void* extra, void* context, void* item)
{
  Match result;
  Match last;

  ((Match)item)->pose = ((RansacContext*)context)->ch->pose;
  result = allocate_match(((PntMatchProblem)extra)->model->size);
  result->pose = (Pose) malloc(sizeof(double) *
			       ((PntMatchProblem)extra)->pose_dim);
  last = copy_match((Match)item);
  last->pose = (Pose) malloc(sizeof(double) *
			       ((PntMatchProblem)extra)->pose_dim);
  iransac_actual(extra,context,last,result);
  ((Match)item)->pose = NULL;
  result->trial_num = ((Match)item)->trial_num;
  free_match((Match)item);
  free_match(last);
  compact_match(result);
  sort_match(result);
  return result; 
}

Match* random_quarter_matches(PntMatchProblem problem, int trials)
{
  Match* matches;
  char* qlist;
  int n;
  
  matches = malloc_array(Match,trials);
  qlist = quarter_pointset(problem->model);
  
  for (n = 0; n < trials; n++) {
    matches[n] = random_quarter_match(problem,qlist);
    matches[n]->trial_num = n;
  }
  free(qlist);
  return matches;
}

void help(void)
{
  int i = 0;
  while (instructions[i]) {
    printf("%s\n",instructions[i]);
    i++;
  }
  printf("\n");
}

int main(int argc, char** argv)
{
  PntMatchProblem problem;
  Match* matches;
  Match* searched;
  unsigned long trials;
  int i;
  clock_t timer;
  clock_t total_rt;
  double seconds;
  list_proc_obj lpo;

  if (argc < 2) {
    help();
    return 0;
  }

  randomize();
  
  /* Load problem description from command line */
  problem = load_problem(argv[1]);
  if (!problem) {
    printf("Problem with problem description file %s.\n",argv[1]);
    exit(1);
  }

  /* A second parameter is the number of trails to run, if available */
  if (argc > 2) trials = atoi(argv[2]);
  else trials = -1;

  /*  Usual practice is to softlink the binary under different names.
      If the program is invoked under a different name, we use that to
      figure out which algorithm to use. Each algorithm has its own default
      number of trials.
  */
  printf("Running on %d processor(s).\n",number_of_processors());

  total_rt = clock();

  if (!strcmp(argv[0],"ransac")) {
    if (trials == -1) trials = 1000;
    timer = clock();
    matches = random_quarter_matches(problem,trials);
    timer = clock() - timer;
    printf("\nRunning %lu trials of RANSAC.\n\n", trials); 
    lpo = get_list_proc_obj((void**)matches,trials,(void*)problem,
			    ransac_wrapper);
    lpo.allocate_scratch_space = init_ransac_context;
    lpo.free_scratch_space = free_ransac_context;

  }
  else if (!strcmp(argv[0],"iransac")) {
    if (trials == -1) trials = 1000;
    timer = clock();
    matches = random_quarter_matches(problem,trials);
    timer = clock() - timer;
    printf("\nRunning %lu trials of iRANSAC.\n\n", trials);
    lpo = get_list_proc_obj((void**)matches,trials,(void*)problem,
			    iransac_wrapper);
    lpo.allocate_scratch_space = init_ransac_context;
    lpo.free_scratch_space = free_ransac_context;

  }

  /* By default, use the key feature algorithm. Note that -1 is the
     default scheme, and 0 asks for half the list. This is controled
     in the key feature routine itself; maybe not the best place for
     it. But having it there saves a processing step and some ram. */
  else if (!strcmp(argv[0],"pntmatch_rs")) {
    if (trials == -1) trials = 1000;
    timer = clock();
    matches = (Match*) malloc(sizeof(Match) * trials);
    for (i = 0; i < trials; i++) 
      matches[i] = random_match(problem->model->size,problem->data->size,
			     problem->min_pairs+1);
    timer = clock() - timer;
    printf("\nRunning %lu trials of random starts local search.\n\n",trials);
    lpo = get_list_proc_obj((void**)matches,trials,(void*)problem,ls_wrapper);
    lpo.allocate_scratch_space = get_search_context;
    lpo.free_scratch_space = free_search_context;
  }

  else {
    timer = clock();
    matches = key_features(problem,problem->min_pairs+1,trials,&trials);
    timer = clock() - timer;
    printf("\nGot %lu key features for local search.\n", trials);
    lpo = get_list_proc_obj((void**)matches,trials,(void*)problem,ls_wrapper);
    lpo.allocate_scratch_space = get_search_context;
    lpo.free_scratch_space = free_search_context;
  }
  
  //Timing report
  seconds = ((double)timer) / ((double)CLOCKS_PER_SEC);
  printf("Took %.3f seconds (%lu clock ticks) to generate all initial starting points.\n",seconds,timer);
  
  //The main event
  timer = clock();
  searched = (Match*) process_list(lpo);
 
  timer = clock() - timer;
  free(matches); matches=searched;
  seconds = ((double)timer) / ((double)CLOCKS_PER_SEC);
  printf("Spent %.3f seconds (%lu clock ticks) searching %lu trials.\n",seconds,
	 timer,trials);
  printf("Average trial time : %.3f seconds.\n",seconds/trials);
  
  //Sort the results
  timer = clock();
  qsort_2t(matches,trials,sizeof(Match),sort_by_trial_num);
  timer = clock() - timer;
  seconds = ((double)timer) / ((double)CLOCKS_PER_SEC);
  printf("Spent %.3f seconds sorting results.\n",seconds);
  
  total_rt = clock() - total_rt;
  seconds = ((double)total_rt) / ((double)CLOCKS_PER_SEC);
  printf("Total algorithm run time : %.3f seconds.\n",seconds);

  report_matches(problem,matches,trials);
  report_matches_html(problem,matches,trials);

  free_problem(problem);
  for (i = 0; i < trials; i++) free_match(matches[i]);
  free(matches);

  return 0;
}
