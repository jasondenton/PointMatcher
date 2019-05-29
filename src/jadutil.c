/**
 * @file jadutil.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdlib.h>
#include <string.h>

/**
 * @brief Free up the memory used by a list of pointers. 
 *
 * free_list calls free on every element of the given list, and then
 * on the list itself.
 *
 * @param list The list to be freed.
 * @param sz The size of the list.
 **/

void free_list(void** list,int sz)
{
  int i;
  if (!list) return;
  for (i = 0; i < sz; i++)
    if (list[i]) free(list[i]);
  free(list);
}

/**
 * @brief Free up the memory used by a null terminated list of
 * pointers.
 *
 * free_ntlist calls free on every element of the given list, until it
 * find the null terminator. Then the list itself is freed.
 *
 *@param list The null terminated list to be freed.
 **/
void free_ntlist(void** list)
{
  int i = 0;
  while(list[i]) {
    free(list[i]);
    i++;
  }
  free(list);
}

#define MAXTOKS 1024

/**
 * @brief Convert a string to a list of tokens. 
 *
 * strtotok splits a given string into a set of tokens, breaking it
 * according to the specified delimiters. Delimiters are not reflected
 * in the output, and if multiple delimiters appear in succession,
 * then no blank field is output.
 *
 * @param str The string to tokenize.
 * @param delim A strong of delimiters.
 * @return A null terminate list of strings.
 **/

char** strtotok(char* str, char* delim)
{
  char** retlist;
  char* list[MAXTOKS];
  char* cstr;
  int idx = 0;
  int i;
  
  cstr = (char*) malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(cstr,str);
  list[0] = strsep(&cstr,delim);
  while (list[idx] != NULL) {
    if (list[idx][0] != '\0') idx++;
    list[idx] = strsep(&cstr,delim);
  }
  
  if (idx == 0) return NULL;
  retlist = (char**) malloc (sizeof(char*) * (idx+1));
  for (i = 0; i < idx; i++) {
    retlist[i] = (char*) malloc(sizeof(char) * (strlen(list[i]) + 1));
    strcpy(retlist[i],list[i]);
  }
  retlist[idx] = NULL;
  free(cstr);
  return retlist;
}


/**
 * @brief Convert a string to a list of tokens.
 *
 * strtofields splits a given string into a set of tokens, breaking it
 * according to the specified delimiters. Delimiters are not reflected
 * in the output, and if multiple delimiters appear in succession,
 * then a blank field is output.
 * @param str The string to tokenize.
 * @param delim A strong of delimiters.
 * @return A null terminate list of strings.
 **/

char** strtofields(char* str, char* delim)
{
  char** retlist;
  char* list[MAXTOKS];
  char* cstr;
  int idx = 0;
  int i;
  
  cstr = (char*) malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(cstr,str);
  list[0] = strsep(&cstr,delim);
  while (list[idx] != NULL) {
    idx++;
    list[idx] = strsep(&cstr,delim);
  }
  
  if (idx == 0) return NULL;
  retlist = (char**) malloc (sizeof(char*) * (idx+1));
  for (i = 0; i < idx; i++) {
    retlist[i] = (char*) malloc(sizeof(char) * (strlen(list[i]) + 1));
    strcpy(retlist[i],list[i]);
  }
  retlist[idx] = NULL;
  free(cstr);
  return retlist;
}

/*
  int main(int argc, char** argv)
  {
  char* test = "::tok1\\tok2 tok3::";
  char* delim = ":\\ ";
  char** list;
  int idx = 0;
  
  list = strtotok(test,delim);
  
  while (list[idx] != NULL) {
  printf("%s\n",list[idx]);
  idx++;
  }
  printf("---------\n");
  free_ntlist(list);
  list = strtofields(test,delim);
  idx = 0;
  while (list[idx] != NULL) {
  printf("%s\n",list[idx]);
  idx++;
  }
  free_ntlist(list);
  
  printf("The factorial of 5 is %d\n",factorial(5));
  return 0;
  }
*/
