/**
 * @file pntmatch.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#ifndef __PNTMATCH__
#define __PNTMATCH__

#include <stdio.h>

typedef double* Pose;

typedef struct {
  int size;
  int allocated;
  double error;
  short* m;
  short* d;
  Pose pose;
  int trial_num;
} MatchData;

typedef MatchData* Match;

#define BAD_MATCH_PENALTY 1e20;

//function prototypes

#ifdef __CPLUSPLUS
extern "C" {
#endif

  void print_match(Match);
  void print_match_html(FILE*,Match);  
  void compact_match(Match);
  void expand_match(Match, int);
  Match allocate_match(int);
  void free_match(Match);
  int compare_match(const void*, const void*);
  void replace_match(Match, Match);
  void sort_match(Match);
  int same_match_instance(Match, Match);
  Match random_match(int,int,int);
  int compatible_matches(Match, Match);
  Match merge_match(Match, Match);
  Match string_to_match(char*);
  Match copy_match(Match);

#ifdef __CPLUSPLUS
}
#endif

#endif
