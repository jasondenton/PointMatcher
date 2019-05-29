/**
 * @file jadmath.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#ifndef _JADMATH_H_
#define _JADMATH_H_

#include <stdio.h>
#include <unistd.h>

#ifdef  __cplusplus
extern "C" {
#endif
  
  int** combinations(int*, int, int, int*);
  int** permutations(int*, int, int*);
  int** permutations_nthings(int*, int, int, int*);
  long factorial(int);
  int ellipsoid(double*t, double* B, double* A, double* b, int m, int n);
	
#ifdef  __cplusplus
}
#endif

#define max(X,Y) ( (X) > (Y) ) ? (X) : (Y)
#define min(X,Y) ( (X) < (Y) ) ? (X) : (Y)

#endif
