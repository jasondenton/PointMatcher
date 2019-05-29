/**
 * @file pproblem.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

/* Sample problem file :
model=testdata.pnt
data=testdata.pnt
sigma=2.0
transform=projective
instances=1
spurious=1
solution=(1,1) (2,2) (3,3)
*/

//the solution field is optional. if present, the solution must not have a 
//newline in it, everything appears on one (very long) line.

#ifndef __PMPROBLEM__
#define __PMPROBLEM__

#include "pntset.h"
#include "pntmatch.h"

typedef struct {
  char* name;
  PointSet model, un_model;
  PointSet data, un_data;
  Match solution;
  double sigma, un_sigma;
  unsigned char transformation;
  double scale;
  char spurious;
  //unsigned char normalized;
  int pose_dim;
  int instances;
  int context_size;
  int context_extra;
  int min_pairs;
  //int context_alloc;
  void (*transform)(double*, double*, Pose);
  double (*degeneracy)(PointSet, Pose, double);
  void (*context_for_pair)(double,double,double,double,double*);
  void (*pose_from_partial)(double*,Pose);
} PntMatchProblemData;

typedef PntMatchProblemData* PntMatchProblem;

typedef struct {
  int pairs; 
  char* paired;
  double* pose;
  double* save;
  double* scratch;
  double* partial;
  double* extra_pose;
  //  double* extra;
} context_handle;

#define TRANSLATION 2
#define RIGID 3
#define SIMILARITY 4
#define AFFINE 6
#define PROJECTIVE 8

#define FULL_EVAL 2e21

#ifdef __CPLUSPLUS
extern "C" {
#endif

  PntMatchProblem load_problem(char*);
  void free_problem(PntMatchProblem);
  int write_problem(FILE*, PntMatchProblem);
  PntMatchProblem read_problem(FILE*,int);
  void print_problem(PntMatchProblem);
  void print_problem_html(FILE*,PntMatchProblem);
  void* get_search_context(void*);
  void free_search_context(void*, void*);
  int initial_context(PntMatchProblem, Match, context_handle*);
  PntMatchProblem inverse_problem(PntMatchProblem);

  //evalution goes here because it requires a problem struct, it is defined
  //in its own file, along with the generic fitting error routine
  //this wrapper also sets the pose (it calls the appropriate pose solver
  //as defined by the problem and stores the results).
  double evaluate_match(PntMatchProblem, Match, double);
  double evaluate_match_with_partial(PntMatchProblem, Match,double, double*);
  PointSet transform_pointset(PointSet,Pose,void (*t)(double*, double*,Pose));
  int model_pose(PntMatchProblem, Match);
  void proper_pose(PntMatchProblem, Match);
  void pose_to_hetro(Pose in, double* out, int dim);
  void print_pose(Pose,int);
  void print_pose_html(FILE*, Pose, int);
  //void print_inverse_pose(Pose,int);
  //void print_inverse_pose_html(FILE*, Pose, int);
  void transform_pointset_inplace(PointSet,Pose,
				  void (*t)(double*, double*, Pose),PointSet);

#ifdef __CPLUSPLUS
}
#endif

#endif
