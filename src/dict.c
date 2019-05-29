/**
 * @file dict.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jaddict.h"
#include "jadutil.h"

#define MAX_PROP_SIZE 8192

/**
 * @brief Read a set of key=value pairs from a file.
 *
 * read_properties reads a file containing a set of key/value pairs
 * and parses the contents, returning a Dictionary (associative array)
 * object.  The file should be formated as a series of lines, with the
 * keys seperated from the values by an equals sign (=). Lines that start
 * with a hash symbol (#) are ignored.
 *
 * @param fname The properties file to read in.
 * @return A dictionary containing the contents of the file.
 **/

Dictionary read_properties(char* fname)
{
  FILE* fin;
  char* input;
  char* part;
  int i,j,idx;
  Dictionary prop;
  
  prop.key = NULL;
  prop.value = NULL;
  prop.size = 0;
  
  fin = fopen(fname,"rt");
  if (!fin) return prop; 
  
  input = malloc_array(char,MAX_PROP_SIZE);
  
  fgets(input,MAX_PROP_SIZE,fin);
  while (!feof(fin)) {
    if (input[0] != '#') prop.size++;
    fgets(input,MAX_PROP_SIZE,fin);	
  }
  
  fseek(fin,0,SEEK_SET);
  prop.key = (char**) malloc(sizeof(char*) * prop.size);
  prop.value = (char**) malloc(sizeof(char*) * prop.size);
  
  prop.size = 0;
  fgets(input,MAX_PROP_SIZE,fin);
  while(!feof(fin)) {
    if (input[0] == '#' || input[0] == ';' || strlen(input) < 3) {
      if (!fgets(input,MAX_PROP_SIZE,fin)) break;
      continue;
    }
    part = strtok(input,"=");
    if (!strcmp(part,"")) continue;
    prop.key[prop.size] = (char*) malloc (sizeof(char)*(strlen(part)+1));
    strcpy(prop.key[prop.size],part);
    part = strtok(NULL,"&\n");
    prop.value[prop.size] = (char*) malloc (sizeof(char)*(strlen(part)+1));
    strcpy(prop.value[prop.size],part);
    prop.size++;
    fgets(input,MAX_PROP_SIZE,fin);
  }
  fclose(fin);
  //bsearch requries a sorted list
  for(i = 0; i < prop.size - 1; i++) {
    idx = i;
    for(j = i + 1; j < prop.size; j++)
      if (strcmp(prop.key[j],prop.key[idx]) < 0) idx = j;
    if (idx != i) { //swap
      part = prop.key[i];
      prop.key[i] = prop.key[idx];
      prop.key[idx] = part;
      part = prop.value[i];
      prop.value[i] = prop.value[idx];
      prop.value[idx] = part;
    }
  }
  free(input);
  return prop;
}

int bsearch_strings(char** strings, char* key, int low, int high)
{
  int mid;
  int res;
  
  if (low > high) return -1;
  
  mid = low + high;
  mid >>= 1;
  
  res = strcmp(key,strings[mid]);
  if (!res) return mid;
  if (res < 0) return bsearch_strings(strings,key,low,mid-1);
  return bsearch_strings(strings,key,mid+1,high);    
}

/**
 * @brief Free up the memory used by a Dictionary object.
 *
 * @param dict The Dictionary to delete.
 **/

void free_dictionary(Dictionary dict)
{
  if (dict.key) free_list((void**)dict.key,dict.size);
  if (dict.value) free_list((void**)dict.value,dict.size);
}

/**
 * @brief Return the value of a given key as a string.
 * 
 * @param prop The dictionary to search.
 * @param key The key to be found.
 * @return The value string associated with key, or NULL if the key is
 * not in the dictionary. The string returned is owned by the dictionary.
 **/

char* get_value_by_key(Dictionary prop, char* key)
{
  int res;
  
  res = bsearch_strings(prop.key,key,0,prop.size-1);
  if (res == -1) return NULL;
  return prop.value[res];
}

/**
 * @brief Return the values associated with a given key.
 *
 * This routine finds the string associated with a given key in a
 * dictionary, and then splits that string on any commas present. The
 * results are returned as a NULL terminated array of strings.
 * 
 * @param prop The dictionary to search.
 * @param key The key to be found.
 * @return The values associated with key, or NULL if the key is
 * not in the dictionary. The returned strings are owned by the caller.
 **/

char** get_array_by_key(Dictionary prop, char* key)
{
  int idx,j,k;
  int i = 0;
  char** list;
  int sz = 1;
  
  idx = bsearch_strings(prop.key,key,0,prop.size-1);
  if (idx == -1) return NULL;
  
  while(prop.value[idx][i]) {
    if (prop.value[idx][i] == ',') sz++;
    i++;
  }
  
  list = (char**) malloc(sizeof(char*) * (sz + 1));
  list[sz] = NULL;
  
  j = 0;
  for (i = 0; i < sz; i++) {
    k = j;
    while(prop.value[idx][j] != ',' && prop.value[idx][j] != '\0') j++;
    j++;
    list[i] = (char*) malloc(sizeof(char) * (j - k));
    strncpy(list[i],prop.value[idx]+k,(j-k-1));    
  }
  
  return list;
}

/**
 * @brief Determine is a dictionary contains all of the specified keys.
 *
 * This routine compares the strings pointed to by contract with the provided
 * dictionary, and returns true if every string appears as a key in the dictionary.
 *
 * @param prop The dictionary to validate.
 * @param contract A null terminated array of strings.
 * @return 1 if every element of contract is a key in prop.
 *         0 Otherwise.
 **/
int validate_contract(Dictionary prop, char** contract)
{
  int i, res;
  
  for (i = 0; contract[i] != NULL; i++) {
    res = bsearch_strings(prop.key,contract[i],0,prop.size-1);
    if (res == -1) return 0;
  }
  return 1;
}
