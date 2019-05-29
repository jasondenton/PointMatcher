/**
 * @file jadimg.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Texas Tech University, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _HAS_LIBJPEG_
#include <jpeglib.h>
#endif

#include "jadimg.h"

COLOR_PIXEL img_red = {255,0,0};
COLOR_PIXEL img_blue = {0,0,255};
COLOR_PIXEL img_green = {0,255,0};
COLOR_PIXEL img_yellow = {213,231,148};
COLOR_PIXEL img_white = {255,255,255};
COLOR_PIXEL img_black = {0,0,0};

//p1 b/w txt
//p2 gray txt
//p3 color txt
//p4 b/w bin
//p5 gray bin
//p6 color bin

/**
 * @brief Load an image from a pxm file.
 *
 * This routine loads an image from a .pbm, .pgm, or .ppm file. The P2 format
 * (black & white binary) is not supported.
 *
 * @param fname The path to the image file.
 * @return The image in the file, or NULL on error.
 **/

IMG img_load_pxm(const char* fname)
{
  FILE* fin;
  IMG image;
  char header[4];
  char width[16];
  char height[16];
  char maxval[6];
  int i,tmp;
  
  fin = fopen(fname,"r"); if (fin == NULL) return NULL;
  fscanf(fin,"%s %s %s %s%*c",header,width,height,maxval);
  
  if (header[0] != 'P') return NULL;
  if (header[1] < '1' || header[1] > '6') return NULL;
  if (atoi(maxval) != 255) return NULL;
  
  image = (IMG) malloc(sizeof(struct _IMGDATA_));
  image->rows = atoi(height);
  image->cols = atoi(width);
  
  switch (header[1]) {
  case '1' :
  case '2' :
    image->color = 0;
    image->pixels.gray = (unsigned char*) malloc(image->rows * image->cols);
    for (i = 0; i < image->rows * image->cols; i++) {
      fscanf(fin,"%d",&tmp);
      image->pixels.gray[i] = tmp;
    }
    break;
  case '4' :
    free(image);
    return NULL;
  case '3' : 
    image->color = 1;
    image->pixels.color = (COLOR_PIXEL*) 
      malloc(sizeof(COLOR_PIXEL) * image->rows * image->cols);
    for (i = 0; i < image->rows * image->cols; i++) {
      fscanf(fin,"%d",&tmp);
      image->pixels.color[i].red = tmp;
      fscanf(fin,"%d",&tmp);
      image->pixels.color[i].green = tmp;
      fscanf(fin,"%d",&tmp);
      image->pixels.color[i].blue = tmp;
    }
    break;
  case '5' :
    image->color = 0;
    image->pixels.gray = (unsigned char*) malloc(image->rows * image->cols);
    fread(image->pixels.gray,1,image->rows * image->cols,fin);
    break;
  case '6' : 
    image->color = 1;
    image->pixels.color = (COLOR_PIXEL*) malloc(sizeof(COLOR_PIXEL) * image->rows * image->cols);
    fread(image->pixels.color,sizeof(COLOR_PIXEL),image->rows * image->cols,fin); 
    break;
  }
  
  fclose(fin);
  return image;
}

/**
 * @brief Write an image to a .pxm format file.
 *
 * This routine writes and image to a designated file. If image is
 * gray scale it is written in P5 format. If it is a color image, it
 * is written in P6 format.
 *
 * @param fname The file to write.
 * @param image The image to write.
 **/

void img_write_pxm(const char* fname, IMG image)
{
  FILE* fout;
  
  fout = fopen(fname,"w");
  if (image->color) fprintf(fout,"P6\n");
  else fprintf(fout,"P5\n");
  fprintf(fout,"%d %d 255\n",image->cols,image->rows);
  
  if (image->color) fwrite(image->pixels.color,sizeof(COLOR_PIXEL),
			   image->rows * image->cols,fout);	
  else fwrite(image->pixels.gray,1,image->rows * image->cols,fout);
  
  fclose(fout);
}

/**
 * @brief Create a new image and allocate space to hold the raster.
 *
 * @param rows The number of rows in the new image.
 * @param cols The number of cols in the new image.
 * @param color 0 for a gray scale image, 1 for color.
 * @return A blank image of the specified type and color.
 **/

IMG img_alloc(int rows, int cols, int color)
{
  IMG image;
  image = (IMG) malloc(sizeof(struct _IMGDATA_));
  image->cols = cols;
  image->rows = rows;
  image->color = color;
  
  if (color)	image->pixels.color = (COLOR_PIXEL*) 
		  malloc(sizeof(COLOR_PIXEL) * rows * cols);
  else image->pixels.gray = (unsigned char*) malloc(rows * cols);
  return image;
}

/**
 * @brief Free up the memory in use by an image.
 * 
 * @param image The image to free.
 **/

void img_free(IMG image)
{
  if (image->color && image->pixels.color != NULL)
    free(image->pixels.color);
  else if (image->color && image->pixels.gray != NULL)
    free(image->pixels.gray);
  free(image);
}

/**
 * @brief Convert a gray scale image to a color format.
 *
 * @param orig The original image.
 * @return A copy of the original image, guarenteed to be in color format.
 **/

IMG img_makecolor(IMG orig)
{
  IMG cimg;
  int i,size;
  
  cimg = img_alloc(orig->rows, orig->cols, 1);
  size = cimg->cols * cimg->rows; 
  for (i = 0; i < size; i++) {
    if (!orig->color) {
      cimg->pixels.color[i].red = orig->pixels.gray[i];
      cimg->pixels.color[i].green = orig->pixels.gray[i];
      cimg->pixels.color[i].blue = orig->pixels.gray[i];
    }	
    else {
      cimg->pixels.color[i].red = orig->pixels.color[i].red;
      cimg->pixels.color[i].green = orig->pixels.color[i].green;
      cimg->pixels.color[i].blue = orig->pixels.color[i].blue;
    }
  }
  
  return cimg;
}

/**
 * @brief Composite two images togther into a new image.
 * 
 * img_composite takes two images, and overlays them according to the
 * given weighting.
 *
 * @param img1 Image 1.
 * @param img2 Image 2.
 * @param weight The weight of the first image, from 0 to 1.
 * @return A composite image.
 **/

IMG img_composite(IMG img1, IMG img2, double weight)
{
  IMG nimg;
  int i,size;
  
  img1 = img_makecolor(img1);
  img2 = img_makecolor(img2);
  nimg = img_alloc(img1->rows,img1->cols,1);
  
  size = img1->rows * img1->cols;
  for (i = 0; i < size; i++) {
    nimg->pixels.color[i].red = img1->pixels.color[i].red * weight;
    nimg->pixels.color[i].red += img2->pixels.color[i].red * (1.0 - weight);
    nimg->pixels.color[i].blue = img1->pixels.color[i].blue * weight;
    nimg->pixels.color[i].blue += img2->pixels.color[i].blue * (1.0 - weight);
    nimg->pixels.color[i].green = img1->pixels.color[i].green * weight;
    nimg->pixels.color[i].green += img2->pixels.color[i].green * (1.0 - weight);
  }
  return nimg;
}

/**
 * @brief Write an image to a JPEG file.
 *
 * This routine writes an image to a jpeg file.  This routine depends
 * on libjpeg being available. To enable this routine, the flag
 * _HAS_LIBJPEG_ must be defined at compile time.
 * @param fname The file to write.
 * @param image The image to write.
 * @return 0 on failure, 1 on success.
 **/

#ifdef _HAS_LIBJPEG_
int img_write_jpg(const char* fname, IMG image)
{
  FILE* fout;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW jrow[1];
  
  //setup jpeg info
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  cinfo.image_width = image->cols;
  cinfo.image_height = image->rows;
  cinfo.input_components = image->color ? 3 : 1;
  cinfo.in_color_space = image->color ? JCS_RGB : JCS_GRAYSCALE;
  jpeg_set_defaults(&cinfo);
  
  //setup output
  if ((fout = fopen(fname,"wb")) == NULL) return 0;
  jpeg_stdio_dest(&cinfo,fout);
  jpeg_start_compress(&cinfo,TRUE);
  
  while (cinfo.next_scanline < cinfo.image_height) {
    if (image->color) 
      jrow[0] = (JSAMPROW) &(image->pixels.color[cinfo.next_scanline *
						 image->cols]);
    else
      jrow[0] = (JSAMPROW) &(image->pixels.gray[cinfo.next_scanline *
						image->cols]);
    
    jpeg_write_scanlines(&cinfo, jrow, 1);
  }
  
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  fclose(fout);
  return 1;
}
#endif
