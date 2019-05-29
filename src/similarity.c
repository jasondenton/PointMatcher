/**
 * @file similarity.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#include <math.h>
#include "pntset.h"
#include "pntmatch.h"

//pose[0] = alpha
//pose[1] = beta
//pose[2] = tx
//pose[3] = ty

#define max(X,Y) (((X) < (Y)) ? (Y) : (X))

void transform_similarity(double* x, double* y, Pose pose)
{
  double tx,ty;
  
  tx = *x * pose[0] - *y * pose[1] + pose[2];
  ty = *x * pose[1] + *y * pose[0] + pose[3];
  *x = tx;
  *y = ty;
}

double degeneracy_similarity(PointSet model, Pose pose, double scale)
{
  double sc;
  
  sc = pose[0] * pose[0] + pose[1] * pose[1];
  sc = sqrt(sc);
  //skipped, because similarity transforms should not be normalized
  //in other words, we assume rescale = 1.0 in all cases
  //sc *= recale;
  sc = max(sc,1.0/sc);
  sc = max(sc-scale,0.0) * ((double)model->size/4.0);
  return sc;
}

void context_for_pair_similarity(double x, double y, double u, double v,
				 double* context)
{
  context[0] = x;
  context[1] = y;
  context[2] = u;
  context[3] = v;
  context[4] = x * u;
  context[5] = y * v;
  context[6] = x * v;
  context[7] = y * u;
  context[8] = x * x + y * y;
  context[9] = 1.0;
}

void pose_from_partial_similarity(double* context, Pose pose)
{
  double denom;

  denom = context[9] * context[8] - (context[0]*context[0]) 
    - (context[1]*context[1]);

  pose[0] = context[9] * (context[4] + context[5]) - (context[0] * context[2]) -
    (context[1] * context[3]);
  pose[0] /= denom;
 
  pose[1] = context[9] * (context[6] - context[7]) + (context[1] * context[2]) -
    (context[0] * context[3]);
  pose[1] /= denom;
 
  pose[2] = -(pose[0] * context[0] - pose[1] * context[1] - context[2]);
  pose[2] /= context[9];
  pose[3] = -(pose[0] * context[1] + pose[1] * context[0] - context[3]);
  pose[3] /= context[9];

  if (pose[0] < 0.000000001 && pose[0] > -0.000000001) pose[0] = 0.0;
  if (pose[1] < 0.000000001 && pose[1] > -0.000000001) pose[1] = 0.0;
  if (pose[2] < 0.000000001 && pose[2] > -0.000000001) pose[2] = 0.0;
  if (pose[3] < 0.000000001 && pose[3] > -0.000000001) pose[3] = 0.0;

}
