/**
 * @file expr_sup.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

//Functions to support experiments

#ifndef __EXPR_SUP__
#define __EXPR_SUP__

#include "pmproblem.h"
#include "jadimg.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

  void report_matches(PntMatchProblem, Match*, int);
  void report_matches_html(PntMatchProblem, Match*, int);
  int sort_by_trial_num(const void*, const void*);
  IMG img_warp_by_pose(double*, IMG,int, int);
  IMG img_markpoints(PointSet, IMG);

#ifdef __CPLUSPLUS
}
#endif

#endif
