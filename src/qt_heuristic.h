/**
 * @file qt_heuristic.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#ifndef _QT_HEUR_H_
#define _QT_HEUR_H_

#include "pntset.h"

#ifdef __CPLUSPLUS
extern "C" {
#endif

  char* quarter_pointset(PointSet);
  Match random_quarter_match(PntMatchProblem, char*);

#ifdef __CPLUSPLUS
}
#endif
#endif


