/**
 * @file matcheqpose.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

/* Copyright 2005 Jason Denton */

#ifndef __MATCHEQPOSE__
#define __MATCHEQPOSE__

#include "pmproblem.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif  

Match closest_match_pairs(PointSet, PointSet, double);
Match pose_to_match(PntMatchProblem, Pose);
Pose* matches_to_poses(PntMatchProblem problem, Match* matches,int);

#ifdef __CPLUSPLUS
}
#endif  

#endif
