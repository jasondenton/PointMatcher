/**
 * @file jadimg.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Texas Tech University, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#ifndef _PXM_H_
#define _PXM_H_

typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} COLOR_PIXEL;

/**
 * @struct _IMGDATA_
 * @brief A raster image.
 *
 * The _IMGDATA_ datatype holds a bitmap image. The raster is stored
 * in row major order. Normally, you use the IMG typedef to refer to a
 * pointer to am image. The jdimg routines refer use a reference
 * design pattern, so _IMGDATA_ is never used directly outside of
 * those routines.
 **/

struct _IMGDATA_ {
  int rows; ///< The number rows in the image.
  int cols; ///< The number of columns in the image.
  char color; ///< 0 for a gray scale image, 1 for color.
    union {       
      COLOR_PIXEL* color;
      unsigned char* gray;
    } pixels;
  /**
     A union holding an array of pixels. Use pixels.gray to
     refer to the array of bytes holding gray scale values, or
     pixels.color to refer to color pixels. Color pixels are themselves
     a struct, with red, green, and blue bytes.
  **/
  
};

/**
 * @brief A reference to an image.
 **/

typedef struct _IMGDATA_* IMG;


/// @brief Macro to refer to the X,Y pixel of color image G.
#define img_color_pixel(G,X,Y) (G->pixels.color[Y * G->cols + X])
/// @brief Macro to refer to the red component of X,Y pixel of color image G.
#define img_red_pixel(G,X,Y) (G->pixels.color[Y * G->cols + X].red)
/// @brief Macro to refer to the blue component of X,Y pixel of color image G.
#define img_blue_pixel(G,X,Y) (G->pixels.color[Y * G->cols + X].blue)
/// @brief Macro to refer to the green component of X,Y pixel of color image G.
#define img_green_pixel(G,X,Y) (G->pixels.color[Y * G->cols + X].green)
/// @brief Macro to refer to the X,Y pixel of gray scale image G.
#define img_gray_pixel(G,X,Y) (G->pixels.gray[Y * G->cols + X])

extern COLOR_PIXEL img_red; ///< Predefined red pixel
extern COLOR_PIXEL img_blue; ///< Predefined blue pixel
extern COLOR_PIXEL img_green; ///< Predefined green pixel
extern COLOR_PIXEL img_yellow; ///< Predefined yellow pixel
extern COLOR_PIXEL img_white; ///< Predefined white pixel
extern COLOR_PIXEL img_black; ///< Predefined black pixel

#ifdef __cplusplus
extern "C" {
#endif

  IMG img_load_pxm(const char*);
  void img_write_pxm(const char*, IMG);
  IMG img_alloc(int, int, int);
  void img_free(IMG);
  IMG img_makecolor(IMG);
  IMG img_composite(IMG,IMG,double);
  int img_write_jpg(const char*, IMG);
#ifdef __cplusplus
}
#endif

#endif
