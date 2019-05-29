/**
 * @file pntset.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#ifndef _PNTSET_H_
#define _PNTSET_H_

#include <stdio.h>

typedef struct {
  int size;
  double* x;
  double* y;
  double lx,ux,ly,uy;
  double length[2];
  char* name;
  char* image;
} PointSetData;

typedef PointSetData* PointSet;


#ifdef __CPLUSPLUS
extern "C" {
#endif

  PointSet load_pointset(char*);
  PointSet allocate_pointset(int);
  double normalize_pointset(PointSet);
  void free_pointset(PointSet);
  void print_pointset(PointSet);
  PointSet copy_pointset(PointSet);
  //int write_pointset(FILE*, PointSet);
  //PointSet read_pointset(FILE*,int);

#ifdef __CPLUSPLUS
}
#endif

#endif
