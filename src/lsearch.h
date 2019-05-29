/**
 * @file lsearch.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/


#ifndef _LSEARCH_H_
#define _LSEARCH_H_

#include "pmproblem.h"

typedef struct {
  char* mtaken;
  char* dtaken;
  double** dist_table;
  PointSet trset;
  context_handle* ch;
} RansacContext;

#ifdef __CPLUSPLUS
extern "C" {
#endif

  int local_search_step(PntMatchProblem, Match, int, context_handle*);
  int local_search(PntMatchProblem, Match, context_handle*);
  //int local_search_one_step(PntMatchProblem, Match);
  Match* key_features(PntMatchProblem, int, long, unsigned long*);
  Match* improved_key_features(PntMatchProblem, int, long,unsigned long*);
  int ransac(PntMatchProblem, Match);
  int iransac(PntMatchProblem, Match);
  void* init_ransac_context(void*);
  void free_ransac_context(void*, void*);
  int ransac_actual(PntMatchProblem,RansacContext*,Match, Match);
  int iransac_actual(PntMatchProblem,RansacContext*,Match, Match);
  unsigned long expected_ransac_trials(PntMatchProblem, double);
  //short** stable_clusters(PointSet, int);

#ifdef __CPLUSPLUS
}
#endif
#endif




