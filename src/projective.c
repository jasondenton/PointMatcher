/**
 * @file projective.c
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

#define max(X,Y) (((X) < (Y)) ? (Y) : (X))
void solvps8(double*,double*);

void transform_projective(double* x, double* y, Pose pose)
{
  double tx,ty;
  double div = 1.0;;

  tx  = *x * pose[0]; ty  = *x * pose[3]; div += *x * pose[6];
  tx += *y * pose[1]; ty += *y * pose[4]; div += *y * pose[7];
  tx += pose[2];      ty += pose[5];      
  *x = tx/div; *y = ty/div;
}

double degeneracy_projective(PointSet model, Pose pose, double scale)
{
  double x[4];
  double y[4];
  double div;
  double length[4];
  int i,j;
  double scterm, vterm;

  vterm = pose[6] * pose[6] + pose[7] * pose[7];
  vterm *= max(model->length[0],model->length[1]);

  //compute the bounding box
  x[0] = pose[0] * model->lx + pose[1] * model->ly + pose[2];
  y[0] = pose[3] * model->lx + pose[4] * model->ly + pose[5];
  div = pose[6] * model->lx + pose[7] * model->ly + 1.0;
  x[0] /= div; y[0] /= div;

  x[1] = pose[0] * model->ux + pose[1] * model->ly + pose[2];
  y[1] = pose[3] * model->ux + pose[4] * model->ly + pose[5];
  div = pose[6] * model->ux + pose[7] * model->ly + 1.0;
  x[1] /= div; y[1] /= div;

  x[2] = pose[0] * model->ux + pose[1] * model->uy + pose[2];
  y[2] = pose[3] * model->ux + pose[4] * model->uy + pose[5];
  div = pose[6] * model->ux + pose[7] * model->uy + 1.0;
  x[2] /= div; y[2] /= div;

  x[3] = pose[0] * model->lx + pose[1] * model->uy + pose[2];
  y[3] = pose[3] * model->lx + pose[4] * model->uy + pose[5];
  div = pose[6] * model->lx + pose[7] * model->uy + 1.0;
  x[3] /= div; y[3] /= div;
  
  for (i = 0; i < 4; i++) {
    //compute the length of each side
    j = (i + 1) % 4;
    div = x[i] - x[j]; div *= div;
    length[i] = div;
    div = y[i] - y[j]; div *= div;
    length[i] += div;
    length[i] = sqrt(length[i]);
    
    //figure relative scale change
    //length[i] *= rescale; //adjust for normalization
    //div against appropriate side length
    //length[i] /= model->length[i % 2];
   
    length[i] *= model->length[i % 2];

    //adjust of shrink or expand
    length[i] = max(length[i], 1.0/length[i]);
    length[i] = max(length[i]-scale,0.0);
  }
  
  scterm = max(length[0],length[1]);
  scterm = max(scterm,length[2]);
  scterm = max(scterm,length[3]);
  //factor 1 scale change beyond allowed equal to omitting 1/4 points
  //this factor is a heuristic; but seems ok.
  scterm *= ((double) model->size)/4.0;

  return scterm + vterm;
}

void context_for_pair_projective(double x, double y,
				 double u, double v, double* context)
{
  double x2,y2,u2,v2,xy;
  
  x2 = x * x;
  y2 = y * y;
  u2 = u * u;
  v2 = v * v;
  xy = x * y;
  u *= -1.0;
  v *= -1.0;
 
  context[0] = x2; //M 0,27
  context[1] = xy; //M 1,8,28,35
  context[2] = x; //M 2,16,29,43
  context[3] = x2 * u; //M 6,48
  context[4] = xy * u; //M 7,14,49,56
  context[5] = y2; //M 9, 36
  context[6] = y; //M 10,17,37,44
  context[7] = y2 * u; //M 15,57
  context[8] = 1.0; //M 18, 45
  context[9] = x * u; //M 22, 50, B0
  context[10] = y * u; //M 23, 58, B1
  context[11] = x2 * v; //M 30,51
  context[12] = xy * v; //M 31, 38, 52, 59
  context[13] = y2 * v; //M 39,60 
  context[14] = x * v; //M 46, 53, B3
  context[15] = y * v; //M 47, 61, B4
  context[16] = x2 * u2 + x2 * v2; //M 54
  context[17] = xy * u2 + xy * v2; //M 55, 62
  context[18] = y2 * u2 + y2 * v2; //M 63
  context[19] = u; //B2
  context[20] = v; //B5
  context[21] = x * u2 + x * v2; //B6
  context[22] = y * u2 + y * v2; //B7
}

void pose_from_partial_projective(double* context, Pose pose)
{
  double* M;
  double* B;
  int i;
  register double tmp;

  M = context + 23;
  B = M + 64;

  tmp = *context;
  M[0] = tmp; M[27] = tmp;

  context++; tmp = *context;
  //M[1] = tmp;
  M[8] = tmp;
  //M[28] = tmp;
  M[35] = tmp;
 
  context++; tmp = *context;
  //M[2] = tmp;
  M[16] = tmp;
  //M[29] = tmp;
  M[43] = tmp;

  context++; tmp = *context;
  //M[6] = tmp;
  M[48] = tmp;

  context++; tmp = *context;
  //M[7] = tmp;
  //M[14] = tmp;
  M[49] = tmp;
  M[56] = tmp;
 
  context++; tmp = *context;
  M[9] = tmp; M[36] = tmp;
  
  context++; tmp = *context;
  //M[10] = tmp;
  M[17] = tmp;
  //M[37] = tmp;
  M[44] = tmp;

  context++; tmp = *context;
  //M[15] = tmp;
  M[57] = tmp;

  context++; tmp = *context;
  M[18] = tmp; M[45] = tmp;

  context++; tmp = *context;
  //M[22] = tmp;
  M[50] = tmp; B[0] = tmp;

  context++; tmp = *context;
  //M[23] = tmp;
  M[58] = tmp; B[1] = tmp;

  context++; tmp = *context;
  //M[30] = tmp;
  M[51] = tmp;

  context++; tmp = *context;
  //M[31] = tmp; M[38] = tmp;
  M[52] = tmp; M[59] = tmp;

  context++; tmp = *context;
  //M[39] = tmp;
  M[60] = tmp;

  context++; tmp = *context;
  //M[46] = tmp;
  M[53] = tmp; B[3] = tmp;
 
  context++; tmp = *context;
  //M[47] = tmp;
  M[61] = tmp; B[4] = tmp;

  context++; 
  M[54] = *context;
  
  context++; tmp = *context;
  //M[55] = tmp;
  M[62] = tmp;

  context++;
  M[63] = *context;
  
  context++; 
  B[2] = *context;
  context++; 
  B[5] = *context;
  context++; 
  B[6] = *context;
  context++; 
  B[7] = *context;
 
  //  M[3] = 0.0; M[4] = 0.0; M[5] = 0.0;
  //M[11] = 0.0; M[12] = 0.0; M[13] = 0.0;
  //M[19] = 0.0; M[20] = 0.0; M[21] = 0.0;

  M[24] = 0.0; M[25] = 0.0; M[26] = 0.0;
  M[32] = 0.0; M[33] = 0.0; M[34] = 0.0;
  M[40] = 0.0; M[41] = 0.0; M[42] = 0.0;

  solvps8(M,B);
  for (i = 0; i < 8; i++) { 
    pose[i] = -B[i];
    if (pose[i] < 0.000000001 && pose[i] > -0.000000001) pose[i] = 0.0;
  }
}
