/**
 * @file pmproblem.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

//Sample problem config file :
//  #problem comment lines start with #
//  model=model.pnt
//  data=data.pnt
//  transform=projective
//  sigma=2.0
//  instances=1
//  spurious=0
//  scale=2.0

#include <stdlib.h>
#include <string.h>
#include "pmproblem.h"
#include "transclass.h"
#include "jadutil.h"
#include "jaddict.h"

void register_transform_class(PntMatchProblem problem)
{
  int normalize = 0;
  double dsc;

  switch(problem->transformation) {
  case PROJECTIVE :
    problem->transform = transform_projective;
    problem->degeneracy = degeneracy_projective;
    problem->pose_from_partial = pose_from_partial_projective;
    problem->context_for_pair = context_for_pair_projective;
    problem->context_size = 23;
    problem->context_extra = 72;
    problem->pose_dim = 8;
    problem->min_pairs = 4;
    normalize = 1;
    break;
  case SIMILARITY :
    problem->transform = transform_similarity;
    problem->degeneracy = degeneracy_similarity;
    problem->context_for_pair = context_for_pair_similarity;
    problem->pose_from_partial = pose_from_partial_similarity;
    problem->pose_dim = 4;
    problem->min_pairs = 2;
    problem->context_size = 10;
    problem->context_extra = 0;
    break;

  default :
    problem->transform = NULL;
    problem->degeneracy = NULL;
    problem->degeneracy = NULL;
    problem->pose_dim = 0;
  }

  // adjust for neccessary normalization
  if (normalize) {
    problem->un_model = copy_pointset(problem->model);
    problem->un_data = copy_pointset(problem->data);
    dsc = normalize_pointset(problem->data);
    problem->sigma *= dsc;
    dsc = normalize_pointset(problem->model) / dsc;
    //if original length on normalized bounding box is l, then when 
    //we transform box into normalized data space,  we expected the length
    //to be ms/ds * l, to account for varying scale changes between the two.
    //since we divide actual length by this value, reversing the terms
    //allows us to mult rather than div later. hack hack hack
    problem->model->length[0] = dsc / problem->model->length[0];
    problem->model->length[1] = dsc / problem->model->length[1];
  }
  else {
    problem->un_model = problem->model;
    problem->un_data = problem->data;
  }
}

PntMatchProblem load_problem(char* fname)
{
  PntMatchProblem problem;
  Dictionary prop;
  char* value;
  char tmp[128];

  prop = read_properties(fname);
  if (prop.size == 0) return NULL;
  problem = (PntMatchProblem) malloc(sizeof(PntMatchProblemData));

  value = get_value_by_key(prop,"transform");
  if (!value) problem->transformation = PROJECTIVE;
  else if (!strcmp(value,"projective")) problem->transformation = PROJECTIVE;
  else if (!strcmp(value,"similarity")) problem->transformation = SIMILARITY;
  else if (!strcmp(value,"affine")) problem->transformation = AFFINE;
  else if (!strcmp(value,"rigid")) problem->transformation = RIGID;
  else if (!strcmp(value,"translate")) problem->transformation = TRANSLATION;
  else { free_dictionary(prop); free(problem); return NULL;}

  value = get_value_by_key(prop,"data");
  if (!value) { free_dictionary(prop); free(problem); return NULL; }
  problem->data = load_pointset(value);
  value = get_value_by_key(prop,"model");
  if (!value) { free_dictionary(prop); free_pointset(problem->data); 
    free(problem); return NULL; }
  problem->model = load_pointset(value);
  
  value = get_value_by_key(prop,"instances");
  if (!value) problem->instances = 1;
  else problem->instances = atoi(value);
  
  value = get_value_by_key(prop,"scale");
  if (!value) problem->scale = 2.0;
  else problem->scale = atof(value);
  
  value = get_value_by_key(prop,"sigma");
  if (!value) problem->sigma = 5.0;
  else problem->sigma = atof(value);
  
  value = get_value_by_key(prop,"spurious");
  if (!value) problem->spurious = 1;
  else problem->spurious = atoi(value);
  
  value = get_value_by_key(prop,"solution");
  if (!value) problem->solution = NULL;
  else problem->solution = string_to_match(value);

  value = get_value_by_key(prop,"name");
  if (!value) {
    sprintf(tmp,"%s_to_%s",problem->model->name,problem->data->name);
    value = tmp;
  }
  problem->name = (char*) malloc(sizeof(char) * (strlen(value) + 1));
  strcpy(problem->name,value);

  problem->un_sigma = problem->sigma;
  register_transform_class(problem);
  problem->sigma *= problem->sigma;

  if (problem->solution) evaluate_match(problem,problem->solution,99999999.99);
  free_dictionary(prop);
  return problem;
}

void free_problem(PntMatchProblem problem)
{
  free_pointset(problem->model);
  free_pointset(problem->data);
  if (problem->model != problem->un_model) free_pointset(problem->un_model);
  if (problem->data != problem->un_data) free_pointset(problem->un_data);
  if (problem->solution) free_match(problem->solution);
  free(problem->name);
  free(problem);
}

void print_problem(PntMatchProblem problem)
{
  char* tclass[] = {"error-0","error-1","translation",
		    "rigid", "similarity", "error-5", "affine", 
		    "error-7","projective"};

  printf("Matching %s to %s\n\n",problem->model->name,problem->data->name);
  printf("%s has %d points\n",problem->model->name,problem->data->size);
  printf("%s has %d points\n",problem->data->name,problem->model->size);
  printf("Transformation class is %s\n",tclass[problem->transformation]);
  printf("Sigma : %5.2f\tAllowed Scaling %5.2f\n",problem->un_sigma,
	 problem->scale);
  printf("%s\n\n", (problem->spurious == 1 ? "Spurious points exist" : 
		    "No spurious points"));
}

void print_problem_html(FILE* fout, PntMatchProblem problem)
{
  char* tclass[] = {"error-0","error-1","translation",
		    "rigid", "similarity", "error-5", "affine", 
		    "error-7","projective"};

  fprintf(fout,"<p><center><strong>%s matched to %s</strong></center><br>\n",
	  problem->model->name,problem->data->name);
  fprintf(fout,
	  "%s has %d points<br>\n",problem->model->name,problem->model->size);
  fprintf(fout,
	  "%s has %d points<br>\n",problem->data->name,problem->data->size);
  fprintf(fout,
	  "Transformation class is %s<br>\n",tclass[problem->transformation]);
  fprintf(fout, "Sigma : %5.2f<br>Allowed Scaling %5.2f<br>\n",
	  problem->un_sigma,problem->scale);
  fprintf(fout,"%s</p>\n", (problem->spurious == 1 ? 
			    "Spurious points exist" : "No spurious points"));
}

void* get_search_context(void* prb)
{
  context_handle* handle;
  int cs;
  void* rc;
  PntMatchProblem problem;

  problem = (PntMatchProblem) prb;

  cs = problem->pose_dim * 2 + problem->context_extra +
	problem->context_size * 3;
  cs *= sizeof(double);
  cs += sizeof(char) * problem->data->size;
  rc = malloc(cs);

  handle = (context_handle*) malloc(sizeof(context_handle));
  handle->pairs = -1;

  handle->extra_pose = (double*) rc;
  handle->pose = handle->extra_pose + problem->pose_dim;
  handle->scratch = handle->pose + problem->pose_dim;
  handle->save = handle->scratch + problem->context_size;
  handle->partial = handle->save + problem->context_size;
  rc = handle->partial + problem->context_size + problem->context_extra;
  handle->paired = (char*) rc;
  return (void*) handle;

}

PntMatchProblem inverse_problem(PntMatchProblem problem)
{
  PntMatchProblem ip;
  short* tmp;

  ip = (PntMatchProblem) malloc(sizeof(PntMatchProblemData));
  ip->transformation = problem->transformation;
  ip->instances = problem->instances;
  ip->scale = problem->scale;
  ip->un_sigma = problem->un_sigma;
  ip->sigma = ip->un_sigma;
  ip->spurious = problem->spurious;
  ip->model = copy_pointset(problem->un_data);
  ip->data = copy_pointset(problem->un_model);
  ip->solution = copy_match(problem->solution);
  if (ip->solution) {
    tmp = ip->solution->m;
    ip->solution->m = ip->solution->d;
    ip->solution->d = tmp;
  }
  ip->name = (char*) malloc(sizeof(char) * (strlen(problem->name)+1));
  ip->name = strcpy(ip->name,problem->name);
  register_transform_class(ip);
  ip->sigma *= ip->sigma;

  if (ip->solution) evaluate_match(ip,ip->solution,99999999.99);
  return ip;
}

void free_search_context(void* foo, void* ch)
{
  //parameter foo is bogus, cause listproc routine wants to provide 
  //the problem handle with the scratch space.
  free(((context_handle*)ch)->extra_pose);
  free(ch);
}

void print_pose(Pose in_pose, int dim)
{
  double r_pose[8];

  pose_to_hetro(in_pose,r_pose,dim);

  printf("Optimal Pose:\n");

  printf("%7.5f ",r_pose[0]);
  printf("%7.5f ",r_pose[1]);
  printf("%7.5f ",r_pose[2]);
  printf("\n");
  printf("%7.5f ",r_pose[3]); 
  printf("%7.5f ",r_pose[4]);
  printf("%7.5f ",r_pose[5]);
  printf("\n"); 
  printf("%7.5f ",r_pose[6]);
  printf("%7.5f ",r_pose[7]); 
  printf("%7.5f ",1.000);
  printf("\n");
}

/*
void print_inverse_pose(Pose in_pose, int dim)
{
  double r_pose[8];

  pose_to_hetro(in_pose,r_pose,dim);

  printf("Optimal Inverse Pose:\n");

  printf("%7.5f ",r_pose[0]);
  printf("%7.5f ",r_pose[1]);
  printf("%7.5f ",r_pose[2]);
  printf("\n");
  printf("%7.5f ",r_pose[3]); 
  printf("%7.5f ",r_pose[4]);
  printf("%7.5f ",r_pose[5]);
  printf("\n"); 
  printf("%7.5f ",r_pose[6]);
  printf("%7.5f ",r_pose[7]); 
  printf("%7.5f ",1.000);
  printf("\n");
}
*/

void print_pose_html(FILE* fout, Pose in_pose, int dim)
{
  double r_pose[8];

  pose_to_hetro(in_pose,r_pose,dim);

  fprintf(fout,"<strong>Optimal Pose</strong><br>\n");
  fprintf(fout,"<table border=\"1\">\n<tr>");

  fprintf(fout,"<td>%7.5f</td>",r_pose[0]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[1]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[2]);
  fprintf(fout,"</tr>\n<tr>");
  fprintf(fout,"<td>%7.5f</td>",r_pose[3]); 
  fprintf(fout,"<td>%7.5f</td>",r_pose[4]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[5]);
  fprintf(fout,"</tr>\n<tr>"); 
  fprintf(fout,"<td>%7.5f</td>",r_pose[6]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[7]); 
  fprintf(fout,"<td>%7.5f</td>",1.000);
  fprintf(fout,"</tr>\n</table>");
}

/*
void print_inverse_pose_html(FILE* fout, Pose in_pose, int dim)
{
  double r_pose[8];

  pose_to_hetro(in_pose,r_pose,dim);

  fprintf(fout,"<strong>Optimal Inverse Pose</strong><br>\n");
  fprintf(fout,"<table border=\"1\">\n<tr>");

  fprintf(fout,"<td>%7.5f</td>",r_pose[0]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[1]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[2]);
  fprintf(fout,"</tr>\n<tr>");
  fprintf(fout,"<td>%7.5f</td>",r_pose[3]); 
  fprintf(fout,"<td>%7.5f</td>",r_pose[4]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[5]);
  fprintf(fout,"</tr>\n<tr>"); 
  fprintf(fout,"<td>%7.5f</td>",r_pose[6]);
  fprintf(fout,"<td>%7.5f</td>",r_pose[7]); 
  fprintf(fout,"<td>%7.5f</td>",1.000);
  fprintf(fout,"</tr>\n</table>");
}
*/
