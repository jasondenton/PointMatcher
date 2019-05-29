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

#ifndef _JDDICT_H_
#define _JDDICT_H_

#include <stdio.h>
#include <unistd.h>

typedef struct {
    char** key;
    char** value;
    int size;
} Dictionary;

#ifdef  __cplusplus
extern "C" {
#endif

  char** get_array_by_key(Dictionary, char*);
  char* get_value_by_key(Dictionary, char*);
  Dictionary read_properties(char*);
  void free_dictionary(Dictionary);

#ifdef  __cplusplus
}
#endif
#endif
