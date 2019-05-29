/**
 * @file expr_sup.c
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
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "pmproblem.h"
#include "jadimg.h"

// Usual error ranking of matches, with that added caveat that 
//matches with a high trial number sort to the bottom of the list.
//This allows the standard compare function to be a true compare,
//and ensures we get propoer credit to the first trial when multiple
//trials produce the same result.
int sort_by_trial_num(const void* e1, const void* e2)
{
	int res;
	Match* m1;
	Match* m2;

	res = compare_match(e1,e2);
	if (res) return res;
	m1 = (Match*) e1;
	m2 = (Match*) e2;
	if ((*m1)->trial_num < (*m2)->trial_num) return -1;
	return 1;
}


void img_fillholes(IMG image, char* fill)
{
    int i,j,k;
    int roffset, offset;
    int surround[8];
    double sum, div;
    
    surround[0] = -(image->cols + 1);
    surround[1] = -image->cols;
    surround[2] = -(image->cols - 1);
    surround[3] = -1;
    surround[4] = 1;
    surround[5] = image->cols - 1;
    surround[6] = image->cols;
    surround[7] = image->cols + 1;

    for (i = 1; i < image->rows - 1; i++) {
	roffset = i * image->cols;
	for (j = 1; j < image->cols - 1; j++) {
	    offset = roffset + j;
	    if (fill[offset]) continue;
	    sum = 0.0;
	    div = 0.0;
	    for (k = 0; k < 8; k++) {
		if (!fill[offset+surround[k]]) continue;
		div += 1.0;
		sum += image->pixels.gray[offset+surround[k]];
	    } //end check surrounding (k)
	    if (div > 0) {
		sum /= div;	       
		image->pixels.gray[offset] = (unsigned char) sum;
	    }

	} // end cols (j)
    } // end rows (i)
}

IMG img_warp_by_pose(double* pose, IMG input,int rows, int cols)
{
    IMG output;
    int i,j;
    int tx,ty;
    double denom;
    char* fill;

    output = img_alloc(rows,cols,0);
    j = rows * cols;
    fill = (char*) malloc(sizeof(char) * j);
    for (i = 0; i < j; i++) {
	output->pixels.gray[i] = 0;
	fill[i] = 0;
    } 

    for (i = 0; i < input->rows; i++)
	for (j = 0; j < input->cols; j++) {
	    denom = pose[6] * j + pose[7] * i + 1.0;
	    tx = (pose[0] * j + pose[1] * i + pose[2])/denom;
	    ty = (pose[3] * j + pose[4] * i + pose[5])/denom;
	    if (tx < 0 || ty < 0) continue;
	    if (tx >= cols || ty >= rows) continue;
	    img_gray_pixel(output,tx,ty) = img_gray_pixel(input,j,i);
	    fill[ty * cols + tx] = 1;
	}
   
    img_fillholes(output,fill);
    return output;
}
IMG img_markpoints(PointSet points, IMG image)
{
  IMG output;
  int p,r,c,x,y;

  output = img_makecolor(image);

  for (p = 0; p < points->size; p++)
    for (r = -1; r <= 1; r++) {
      y = (int) round(points->y[p] + r);
      if (y < 0 || y >= output->rows) continue;   
      for (c = -1; c <= 1; c++) {
	x = (int) round(points->x[p] + c);
	if (x < 0 || x >= output->cols) continue;
	if (r == 0 && c == 0) img_color_pixel(output,x,y) = img_black;
	else img_color_pixel(output,x,y) = img_white;
      } 
    }

  return output;
}

void report_matches(PntMatchProblem problem,Match* matches, int trials)
{
  int i, k, firsti;
  // int with_images = 1;
  //char buf[80];
  //PXM mimg,dimg,nimg,timg;

  printf("\n");
  print_problem(problem);

  //load data and model images in preperation for outputing
  //warped version of found set.
  //Probably ought to error check for presence of original/data
  //images here.
 
  /*
 if (problem->model->image == NULL || problem->data->image == NULL)
    with_images = 0;

  if (with_images) {
    mimg = img_load_pxm(problem->model->image);
    dimg = img_load_pxm(problem->data->image);
  }
  */
  
  if (problem->solution) {
    proper_pose(problem,problem->solution);
    printf("***** Previously known good solution *****\n");
    print_match(problem->solution); 
    /*
      for (j = 0; j < problem->pose_dim; j++) {
      printf("%c: %8.6f ",'a'+j,matches[firsti]->pose[j]);
      if (j > 0 && (j+1) % 3 == 0) printf("\n");
    }
    */
    print_pose(problem->solution->pose,problem->pose_dim);
    printf("\n\n");


  }

  //This loop here kicks out matches and corresponding pictures for
  //every match that is within 0.5 of the optimal match.
  //Probably ought to make this honor the "instances" field, but
  //we are also interested in what might be "close"
  i = 0; k = 1; 
  while (k <= problem->instances && i < trials) {
    firsti = i;
    //find the next instance of this object in the scene
    //this may not be the next match, if the next match
    //is similar to the match above it on the match list.
    if (i > 0 && same_match_instance(matches[i],matches[i-1]))
      { i++; continue; }
    
    //We may have normalized the point set positions as part of problem
    //setup.
    proper_pose(problem,matches[firsti]);
    printf("***** Result %d found on trial %d *****\n",k,
	   matches[firsti]->trial_num+1);
    print_match(matches[firsti]); 
    /*
      for (j = 0; j < problem->pose_dim; j++) {
      printf("%c: %8.6f ",'a'+j,matches[firsti]->pose[j]);
      if (j > 0 && (j+1) % 3 == 0) printf("\n");
    }
    */
    print_pose(matches[firsti]->pose,problem->pose_dim);
    printf("\n\n");
    
    /*
    if (with_images) {
      timg = img_warp_by_pose(matches[firsti]->pose,mimg,dimg.rows,dimg.cols);
      nimg = img_composite(timg,dimg,0.5);
      sprintf(buf,"result%d.ppm",k);
      img_write(buf,nimg);
      img_free(timg);
      img_free(nimg);
    }
    */
    i++; k++;
  }
  
  //cleanup. Not strictly neccessary on modern systems, but deleting 
  //stuff before program termination is a good way to catch pointer bugs.
  /*
    if (with_images) {
    img_free(mimg);
    img_free(dimg);
    }
  */
}

void report_matches_html(PntMatchProblem problem,Match* matches, int trials)
{
  int i, k, firsti;
  int with_images = 1;
  char buf[80];
  char destdir[80];
  IMG mimg,dimg,nimg,timg;
  FILE* fout;

  mimg = NULL;
  dimg = NULL;
  nimg = NULL;
  timg = NULL;

  sprintf(destdir,"results_%s",problem->name);
  mkdir(destdir,0700);
  sprintf(buf,"%s/index.html",destdir);
  fout = fopen(buf,"wt");
  fprintf(fout,"<html>\n<head><title>");
  fprintf(fout,"%s matched to %s",problem->model->name,problem->data->name);
  fprintf(fout,"</title></head>\n");
  fprintf(fout,"<body>\n");
	    
  print_problem_html(fout,problem);
  fprintf(fout,"<hr>\n");

  //load data and model images in preperation for outputing
  //warped version of found set.
  //Probably ought to error check for presence of original/data
  //images here.
  if (problem->model->image == NULL || problem->data->image == NULL)
    with_images = 0;

  if (with_images) {
    mimg = img_load_pxm(problem->model->image);
    sprintf(buf,"%s/model.jpg",destdir);
    img_write_jpg(buf,mimg);
    dimg = img_load_pxm(problem->data->image);
    sprintf(buf,"%s/data.jpg",destdir);
    img_write_jpg(buf,dimg);

    fprintf(fout,"<p><center><table><tr><td>Model</td><td>Data</td></tr>\n");
    fprintf(fout,"<tr><th><img src=\"model.jpg\"></th>\n");
    fprintf(fout,"<th><img src=\"data.jpg\"></th></tr>\n");
    fprintf(fout,"</table></center></p>\n");
  }

  //This chunk reports on any previously known solution
  if (problem->solution) {
    proper_pose(problem,problem->solution);
    fprintf(fout,
	    "<hr>\n<p><center><strong>Previously known good solution</center></strong>\n");

    fprintf(fout,"<p><center><table><tr><td>");
    if (with_images) {
      timg = img_warp_by_pose(problem->solution->pose,mimg,dimg->rows,dimg->cols);
      nimg = img_composite(timg,dimg,0.5);
      fprintf(fout,"<img src=\"known_solution.jpg\">\n"); 
      sprintf(buf,"%s/known_solution.jpg",destdir);
      img_write_jpg(buf,nimg);
      img_free(timg);
      img_free(nimg);
    }
    fprintf(fout,"</td><td>");
    print_pose_html(fout,problem->solution->pose,problem->pose_dim);
    fprintf(fout,"</td></tr></table></center></p>");
    print_match_html(fout,problem->solution); 
  }

  //This loop here kicks out matches and corresponding pictures for
  //every match that is within 0.5 of the optimal match.
  //Probably ought to make this honor the "instances" field, but
  //we are also interested in what might be "close"
  i = 0; k = 1; 
  while (k <= problem->instances && i < trials) {
    firsti = i;
    //find the next instance of this object in the scene
    //this may not be the next match, if the next match
    //is similar to the match above it on the match list.
    if (i > 0 && same_match_instance(matches[i],matches[i-1]))
      { i++; continue; }
    
    //We may have normalized the point set positions as part of problem
    //setup.
    proper_pose(problem,matches[firsti]);
    fprintf(fout,"<hr>\n");
    fprintf(fout,
	    "<p><center><strong>Result %d found on trial %d</center></strong>\n",k,matches[firsti]->trial_num+1);

    fprintf(fout,"<p><center><table><tr><td>");
    if (with_images) {
      timg = img_warp_by_pose(matches[firsti]->pose,mimg,dimg->rows,dimg->cols);
      nimg = img_composite(timg,dimg,0.5);
      sprintf(buf,"result%d.jpg",k);
      fprintf(fout,"<img src=%s>\n",buf); 
      sprintf(buf,"%s/result%d.jpg",destdir,k);
      img_write_jpg(buf,nimg);
      img_free(timg);
      img_free(nimg);   
    }
    fprintf(fout,"</td><td>");
    print_pose_html(fout,matches[firsti]->pose,problem->pose_dim);
    fprintf(fout,"</td></tr></table></center></p>");
    print_match_html(fout,matches[firsti]); 
    i++; k++;
  }
  
  //cleanup. Not strictly neccessary on modern systems, but deleting 
  //stuff before program termination is a good way to catch pointer bugs.
  if (with_images) {
    img_free(mimg);
    img_free(dimg);
  }
  fprintf(fout,"</body>\n</html>");
  fclose(fout);
}
