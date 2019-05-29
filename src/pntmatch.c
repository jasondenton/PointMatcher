/**
 * @file pntmatch.c
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
#include "pntmatch.h"
#include "random.h"
#include "jadutil.h"
#include "jadmath.h"

void print_match(Match match)
{
  int i,j;

  j = 0;
  for (i = 0; i < match->size; i++) {
    if (match->m[i] == -1) continue;
    if (match->d[i] == -1) continue;
    j++;
    printf("(%3d,%3d) ",match->m[i],match->d[i]);
   if (j % 8 == 0) printf("\n"); 
 }
  printf("\nPairs: %d Fitness: %8.4f\n",j,match->error);
}

void print_match_html(FILE* fout, Match match)
{
  int i,j;

  j = 0;
  fprintf(fout,"<p><table>\n");
  for (i = 0; i < match->size; i++) {
    if (match->m[i] == -1) continue;
    if (match->d[i] == -1) continue;
    j++;
    if (j % 8 == 1) fprintf(fout,"<tr>");
    fprintf(fout,"<td>(%3d,%3d)</td>",match->m[i],match->d[i]);
    if (j % 8 == 0) fprintf(fout,"</tr>\n");
  }
  fprintf(fout,"</table><br>\n");
  fprintf(fout,"\nPairs: %d Fitness: %8.4f</p>\n",j,match->error);
}

void compact_match(Match match)
{
  int count = 0;
  int i;
  short *m;
  short *d;

  for (i = 0; i < match->size; i++)
    if (match->m[i] != -1 && match->d[i] != -1) count++;
    
  
  if (count == match->allocated) return;
  
  m = (short*) malloc(sizeof(short) * count);
  d = (short*) malloc(sizeof(short) * count);

  count = 0;
  for (i = 0; i < match->size; i++) {
    if (match->m[i] == -1) continue;
    if (match->d[i] == -1) continue;
    m[count] = match->m[i];
    d[count] = match->d[i];
    count++;
  }
  if (match->allocated > 0) {
    free(match->m);
    free(match->d);
  }
  match->m = m; 
  match->d = d;
  match->allocated = count;
  match->size = count;
  if (match->pose) free(match->pose);
  match->pose = NULL;
}

void expand_match(Match match, int ms)
{
  short* m;
  short* d;
  int i;
  
  m = (short*) malloc(sizeof(short) * ms);
  d = (short*) malloc(sizeof(short) * ms);
  
  for (i = 0; i < ms; i++) {
    m[i] = i; 
    d[i] = -1;
  }
  for (i = 0; i < match->size; i++) 
    if (match->d[i] != -1) d[match->m[i]] = match->d[i];
  if (match->allocated > 0) {
    free(match->m);
    free(match->d);
  }
  match->m = m; 
  match->d = d;
  match->size = ms;
  match->allocated = ms;
}

Match allocate_match(int alloc)
{
  Match match;
  int i;

  match = (Match) malloc(sizeof(MatchData));

  match->allocated = alloc;
  match->size = 0;
  match->error = 0.0;
  match->pose = NULL;
  match->m = (short*) malloc(sizeof(short) * alloc);
  match->d = (short*) malloc(sizeof(short) * alloc);

  for(i = 0; i < alloc; i++) {
    match->m[i] = -1;
    match->d[i] = -1;
  }

  return match;
}

void free_match(Match match)
{
  if (!match) return;
  if (match->allocated > 0) {
    free(match->m);
    free(match->d);
  }
  if (match->pose) free(match->pose);
  free(match);
}

int compare_match(const void* m1, const void* m2)
{
  int i, nsame, ret;
  double diff;
  Match* match1;
  Match* match2;

  // deal with possiblity that match is null
  // null matches always greater than an instantiated match
  // helps with key feature ranking/memory saving scheme
  // by dumping null matches at end of list
  
  match1 = (Match*) m1;
  match2 = (Match*) m2;

  //deal with null match
  if (!(*match1) && !(*match2)) return 0;
  if (!(*match1)) return 1;
  if (!(*match2)) return -1;

  //set my return code based on absolute error
  //we MIGHT override if it looks like the difference
  //is just fp error.
  if ((*match1)->error < (*match2)->error) ret = -1;
  else if ((*match1)->error > (*match2)->error) ret =  1;
  else ret = 0;

  //this is not exactly accurate, it assumed the
  //all model points appear in acesending order.
  //this should be the case in expanded format.
  //it should also be the case when an expanded match
  //gets passed through compact_match.
  diff = fabs((*match1)->error - (*match2)->error);
  if (diff < 0.005) {
    i = 0; nsame = 0;
    while (i < (*match1)->size && !nsame) {
      if ((*match1)->m[i] != (*match2)->m[i]) nsame = 1;
      if ((*match1)->d[i] != (*match2)->d[i]) nsame = 1;
      i++;
    } 
    if (!ret && nsame) ret = (*match1)->size < (*match2)->size ? -1 : 1;
    ret = ret * nsame;
  }
  return ret;
}

//Match is a reference type. Calls like ransac might completely
//replace the contents of the match. This need to be reflected in the
//reference.
void replace_match(Match match, Match rep)
{
  if (match->allocated > 0) {
    free(match->m);
    free(match->d);
  }
  match->allocated = rep->allocated;
  match->m = rep->m;
  match->d = rep->d;

  match->error = rep->error;
  match->size = rep->size;
  
  //problem is, pose is not something the match really owns itself it all
  //cases. So leave pose untouched.
  if (rep->pose) free(rep->pose);
  free(rep);
}

//sort pairs by model point
void sort_match(Match match)
{
  int i,j;
  int min;

  for (i = 0; i < match->size - 1; i++) {
    min = i;
    for (j = i + 1; j < match->size; j++) 
      if (match->m[j] < match->m[min]) min = j;
    if (min != i) {
      match->m[i] ^= match->m[min];
      match->m[min] ^= match->m[i];
      match->m[i] ^= match->m[min];
      match->d[i] ^= match->d[min];
      match->d[min] ^= match->d[i];
      match->d[i] ^= match->d[min];
    }
  }
}

//This is a sort of compare function. It checks to see
//if the matches in question share 50% or more of their
//pairs. If they do, they are the same match "instance",
//meaning they represent the same object in the scence.
int same_match_instance(Match m1, Match m2)
{
  int i,j,found;
  int same_pairs = 0;
  int m1pairs = 0;
  int m2pairs = 0;

  for (i = 0; i < m1->size; i++) {
    if (m1->d[i] == -1) continue;
    m1pairs++;
    found = 0;
    j = 0;
    while (!found && j < m2->size) {
      if (m1->m[i] == m2->m[j] && m1->d[i] == m2->d[j])
	found = 1;
      j++;
    }
    same_pairs += found;
  }

  for(i = 0; i < m2->size; i++)
    if (m2->d[i] == -1) continue;
    else m2pairs++;

  m1pairs = max(m1pairs,m2pairs);
  return (same_pairs > m1pairs*0.75);
}

Match random_match(int ms, int ds, int sz)
{
  Match match;
  int i;
  char* mt;
  char* dt;
  int np;

  match = allocate_match(sz);
  mt = (char*) malloc(sizeof(char) * ms);
  dt = (char*) malloc(sizeof(char) * ds);
  
  for (i = 0; i < ms; i++) mt[i] = 1;
  for (i = 0; i < ds; i++) dt[i] = 1;

  for (i = 0; i < sz; i++) {
    np = randint(ms);
    while (!mt[np]) np = (np + 1) % ms;
    mt[np] = 0;
    match->m[i] = np;
    np = randint(ds);
    while (!dt[np]) np = (np + 1) % ds;
    dt[np] = 0;
    match->d[i] = np;
  }
  match->size = sz;
  return match;
}

//returns true if two matches are compatible, false if they are not.
//matches are compatible if they do not disagree about which model/data pairs
//are correct. In other terms, they can be combined to produce a
//(possibly) longer match while still maintaining the 1 to 1 constraint.
//Requires match to be in long form.
int compatible_matches(Match m1, Match m2)
{

  int i,j;
  int compat = 1;

  for (i = 0; i < m1->size && compat; i++) {
    if (m1->d[i] == -1) continue;
    for (j = 0; j < m2->size && compat; j++) {
      if (m2->d[j] == -1) continue;
      if (m1->m[i] == m2->m[j] && m1->d[i] != m2->d[j]) compat = 0;
      if (m1->d[i] == m2->d[j] && m1->m[i] != m2->m[j]) compat = 0;
    }
  }
   
  return compat;
}

//take two partial matches, merge to create one new match. originals preserved.
//originals must be in long form
Match merge_match(Match m1, Match m2)
{
  Match m3;
  int i;

  if (!compatible_matches(m1,m2)) return NULL;

  m3 = allocate_match(m1->size);
  
  for(i = 0; i < m1->size; i++) {
    m3->m[i] = m1->m[i];
    m3->d[i] = m1->d[i];
  }
  m3->size = m1->size;

  //m3 now contains a copy of m1. Assuming long form, we need only copy
  //over data pairings
  for (i = 0; i < m2->size; i++) 
    if (m2->d[i] != -1) m3->d[i] = m2->d[i];
  
  return m3;
}

Match string_to_match(char* str)
{
  Match match;
  int sz = 0;
  char** tok;
  int i;

  tok = strtotok(str,"(,) \n");
  while (tok[sz]) sz++;
  if (sz %2) return NULL;
  sz /= 2;
  
  match = allocate_match(sz);
  match->size = sz;
  for (i = 0; i < sz; i++) {
    match->m[i] = atoi(tok[i*2]);
    match->d[i] = atoi(tok[i*2+1]);
  }
  free_ntlist((void**)tok);
  sort_match(match);
  return match;
}

Match copy_match(Match match)
{
  Match nm;
  int i;

  if (!match) return NULL;
  nm = allocate_match(match->size);

  for (i = 0; i < match->size; i++) {
    nm->m[i] = match->m[i];
    nm->d[i] = match->d[i];
  }
  nm->size = match->size;
  nm->error = match->error;
  nm->trial_num = match->trial_num;
  return nm;
}
