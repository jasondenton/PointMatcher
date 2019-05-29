/**
 * @file transclass.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

// This header file is used internally by pmproblem.c to register its
// transformation classes. Generally, it should contain only the prototypes
// for the functions defining a transformation class.

// To add a transformation class, write the appropriate three functions
// (transform, degeneracy, and pose determination), place their prototypes
// here, and modify pmproblem.c to understand the new class. A #define
// may also be needed in pmproblem.h

#include "pntset.h"
#include "pntmatch.h"

#ifdef __cplusplus
extern "C" {
#endif

void transform_projective(double*, double*, Pose);
double degeneracy_projective(PointSet, Pose, double);
void context_for_pair_projective(double, double, double, double, double*);
void pose_from_partial_projective(double*, Pose);

void transform_similarity(double*, double*, Pose);
double degeneracy_similarity(PointSet, Pose, double);
void context_for_pair_similarity(double, double, double, double, double*);
void pose_from_partial_similarity(double*, Pose);


void transform_affine(double*, double*, Pose);
double degeneracy_affine(PointSet, Pose, double);
void context_for_pair_affine(double, double, double, double, double*);
void pose_from_partial_affine(double*, Pose);


#ifdef __cplusplus
}
#endif
