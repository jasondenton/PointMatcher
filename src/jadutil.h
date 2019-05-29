/**
 * @file jadutil.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#ifndef _JDUTIL_H_
#define _JDUTIL_H_

#include <stdio.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif
  
  void free_list(void**,int);
  void free_ntlist(void**);
  char** strtotok(char*, char*);
  char** strtofields(char*, char*);
  
#ifdef  __cplusplus
}
#endif

#define malloc_array(X,Y) (X*) malloc(sizeof(X) * (Y))

#endif
