/**
 * @file pntset.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pntset.h"

void set_pointset_auxdata(PointSet);

/* Print out a loaded point set */
void print_pointset(PointSet data)
{
  int i;
  printf("Point Set: %s\n",data->name);
  for (i = 0; i < data->size; i++)
    printf("%d: %lf\t %lf\n",i,data->x[i],data->y[i]);
  printf("\n");
}

double normalize_pointset(PointSet points)
{
  double cx,cy;
  int i;
  
  // variables for re-scale
  double sc;
  double avg_dist = 0.0;
  
  // find bounding corners
  points->lx = points->x[0]; points->ux = points->x[0];
  points->ly = points->y[0]; points->uy = points->y[0];
  
  for (i = 1; i < points->size; i++) {
    if (points->x[i] < points->lx) points->lx = points->x[i]; 
    else if (points->x[i] > points->ux) points->ux = points->x[i]; 
    if (points->y[i] < points->ly) points->ly = points->y[i]; 
    else if (points->y[i] > points->uy) points->uy = points->y[i]; 
  }
  
  //move center to origin
  cx = (points->ux - points->lx)/2.0 + points->lx;
  cy = (points->uy - points->ly)/2.0 + points->ly;   
  for(i = 0; i < points->size; i++) {
    points->x[i] -= cx;
    points->y[i] -= cy;
  }
  
  //scale change starts here
  for(i = 0; i < points->size; i++) {
    cx = points->x[i] * points->x[i];
    cy = points->y[i] * points->y[i];
    avg_dist += sqrt(cx + cy);
  }
  sc = (sqrt(2.0) * (double) points->size) / avg_dist;
  
  //two loops, because we expect operations to be vectorized
  //probably not worth fixing on a non-vectorizing compilier
  for(i = 0; i < points->size; i++) points->x[i] *= sc;
  for(i = 0; i < points->size; i++) points->y[i] *= sc;
  
  set_pointset_auxdata(points);
  
  return sc;
}

void set_pointset_auxdata(PointSet points)
{
  int i;
  
  // find bounding corners, again
  points->lx = points->x[0]; points->ux = points->x[0];
  points->ly = points->y[0]; points->uy = points->y[0];
  
  for (i = 1; i < points->size; i++) {
    if (points->x[i] < points->lx) points->lx = points->x[i]; 
    else if (points->x[i] > points->ux) points->ux = points->x[i]; 
    if (points->y[i] < points->ly) points->ly = points->y[i]; 
    else if (points->y[i] > points->uy) points->uy = points->y[i]; 
  }
  
  //          0
  //  lx,ly ----- ux,ly
  //    |           |
  //  3 |           | 1
  //    |           |
  //  lx,uy ------ux,uy
  //          2
  
  points->length[0] = points->ux - points->lx;
  points->length[1] = points->uy - points->ly;
}


PointSet allocate_pointset(int size)
{
  PointSet points;
  
  points = (PointSet) malloc(sizeof(PointSetData));
  
  points->size = size;
  
  if (size == 0) {
    points->x = NULL;
    points->y = NULL;
  }
  else {
    points->x = (double*) malloc(sizeof(double) * size);
    points->y = (double*) malloc(sizeof(double) * size);
  }
  
  points->name = NULL;
  points->image = NULL;
  points->lx = 0.0; points->ux = 0.0;
  points->ly = 0.0; points->uy = 0.0;
  points->length[0] = 0.0;
  points->length[1] = 0.0;
  return points;
}

void free_pointset(PointSet points)
{
  if (points->x) free(points->x);
  if (points->y) free(points->y);
  if (points->name) free(points->name);
  if (points->image) free(points->image);
  free(points);
}

PointSet load_pointset(char* fname)
{
  PointSet points;
  FILE* fin;
  char str[96];
  int i, sz;
  char* tok;
  char* name = NULL;
  char* image = NULL;

  if (!(fin = fopen(fname,"rt"))) return NULL;
  
  /* Count the number of points in the file */
  sz = 0;
  while (fgets(str,96,fin) != NULL) 
    if (str[0] == '#') {
      tok = strtok(str," ");
      if (!strcmp(tok,"#image")) {
	tok = strtok(NULL,"\n");
	image = strdup(tok);
      }
      else if (!strcmp(tok,"#name")) {
	tok = strtok(NULL,"\n");
	name = strdup(tok);
      }
    }
    else if (str[0] != '\n') sz++;
  
  /* Allocate memory and read in the x and y position of each point */
  points = allocate_pointset(sz);
  points->name = name;
  points->image = image;

  rewind(fin); i = 0;
  for (i = 0; i < points->size; i++) {
    fgets(str,96,fin);
    if (str[0] == '#') {i--; continue;}
    if (str[0] == '\n') {i--; continue;}
    sscanf(str,"%lf %lf",points->x+i,points->y+i);
  }
  fclose(fin); 

  set_pointset_auxdata(points);

  return points;
}

PointSet copy_pointset(PointSet pset)
{
  PointSet copy;
  int i;

  copy = (PointSet) malloc(sizeof(PointSetData));
  copy->size = pset->size;
 
  if (copy->size == 0) { copy->x = NULL; copy->y = NULL; }
  else {
    copy->x = (double*) malloc(sizeof(double) * copy->size);
    copy->y = (double*) malloc(sizeof(double) * copy->size);
  }
  
  if (pset->name) {
    copy->name = (char*) malloc(sizeof(char) * (strlen(pset->name) + 1));
    strcpy(copy->name,pset->name);
  }
  else copy->name = NULL;

  if (pset->image) {
    copy->image = (char*) malloc(sizeof(char) * (strlen(pset->image) + 1));
    strcpy(copy->image,pset->image);
  }
  else copy->image = NULL;

  for (i = 0; i < copy->size; i++) {
    copy->x[i] = pset->x[i];
    copy->y[i] = pset->y[i];
  }
  copy->lx = pset->lx; copy->ux = pset->ux;
  copy->ly = pset->ly; copy->uy = pset->uy;
  copy->length[0] = pset->length[0];
  copy->length[1] = pset->length[1];

  return copy;
}
