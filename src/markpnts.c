/**
 * @file markpnts.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#include "pntset.h"
#include "jadimg.h"

IMG img_markpoints(PointSet, IMG);

int main(int argc, char** argv)
{
  IMG image;
  PointSet pset;
  
  image = img_load_pxm(argv[1]);
  pset = load_pointset(argv[2]);

  image = img_markpoints(pset,image);
  img_write_pxm(argv[3],image);

  return 0;
}
