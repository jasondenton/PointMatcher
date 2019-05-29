/**
 * @file combinations.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Return the factorial of some number.
 * @param fact An integer.
 * @returns The factorial of fact.
 **/

unsigned long factorial(unsigned short fact)
{
  int i;
  unsigned long f = 1;
  
  if (fact < 1) return 1;
  for (i = 2; i <= fact; i++)
    f *= i;
  return f;
}

/**
 * @brief Determine all possible combinations for some list of
 * elements.
 *
 * combinations computes the list of possible combinations of size
 * num, for the list of integers provided to it. For instance, if the
 * list contains the elements {10, 15, 23} and you want all
 * combinations of size 2, then the returned list of lists would
 * contains {{10,15},{10,23},{15,23},NULL}. Ordering of elements is
 * not guarenteed.
 *
 * The number of possible combinations grows very quickly, and for any
 * significant combination of list size and desired elements per
 * combination this function is likely to run out of memory and
 * crash. Since the function itself is recursive, overflowing the
 * stack is also a real possiability. This routine is not
 * particularlly efficent. Those with a serious need for this
 * functionality should go read Knuth, Art of Programming, vol 3.
 *
 * @param things A list of integers
 * @param num_el Size of the integer list
 * @param num The number of elements desired in each combination
 * @param sz[out] The number of combinations returned by the function. If
 * set to NULL then this information is not returned.
 * @return A list of combination lists. Each combination list is of
 * size num and is NOT null terminated. The list of combinations
 * itself is NULL terminated.
 **/

int** combinations(int* things, int num_el, int num, int* sz)
{
    int** nlist;
    int*** llist;
    int i,j,k,l;
    int nc,lsz;


    //for combinations of one thing, just break the list apart
    //and return it
    if (num > num_el) return NULL;
    if (num == 1) {
	nlist = (int**) malloc(sizeof(int*) * (num_el + 1));
	for (i = 0; i < num_el; i++) {
	    nlist[i] = (int*) malloc(sizeof(int));
	    nlist[i][0] = things[i];
	}
	nlist[num_el] = NULL;
	if (sz) *sz = num_el;
	return nlist;
    }

    //allocate space for a list of lists
    llist = (int***) malloc(sizeof(int**) * (num_el-1));
    lsz = 0;
    for (i = 1; i < num_el; i++) {
	//each list is the combinations one size smaller
	//keep a running total of lists in the list of lists
	llist[i-1] = combinations(things+i,num_el-i,num-1,&nc);
	lsz += nc;
    }
    llist[num_el-2] = NULL; 
    
    //allocate space for the list we will return
    nlist = (int**) malloc(sizeof(int*) * lsz);

    //copy the list of list out to the new list, appending things as needed
    l = 0;
    i = 0;
    while(llist[i] != NULL) {
	j = 0;
	while(llist[i][j] != NULL) {
	    nlist[l] = (int*) malloc(sizeof(int) * num);
	    nlist[l][0] = things[i];
	    for (k = 0; k < num-1; k++)
		nlist[l][k+1] = llist[i][j][k];
	    l++;
	    j++;
	}
	i++;
    }
    if (sz) *sz = l;
    nlist[l] = NULL;
    return nlist;
}


/**
 * @brief Determine all possible permutations (orderings) for a list of
 * things.
 *
 *permutations computes the list of possible permutations of some
 * list. Ordering of elements in the returned list is not guarenteed.
 *
 * The number of possible permutations grows very quickly, and for any
 * significant list this function is likely to run out of memory and
 * crash. Since the function itself is recursive, overflowing the
 * stack is also a real possiability. This routine is not
 * particularlly efficent. Those with a serious need for this
 * functionality should go read Knuth, Art of Programming, vol 3.
 *
 * @param list A list of integers
 * @param size Size of the integer list
 * @param sz[out] The number of permutations returned by the function. If
 * set to NULL then this information is not returned.
 * @return A list of permutation lists. Each permutation list is of
 * size size and is NOT null terminated. The list of permutations
 * itself is NULL terminated.
 **/

int** permutations(int* list, int size, int* sz)
{
    int** nlist;
    int** olist;
    int i,j,k;
    int lpos;
    int osize;
    if (size <= 0) {if (*sz) sz = 0; return NULL;}
    if (size == 1) {
	nlist = (int**) malloc(sizeof(int*) * 2);
	nlist[0] = (int*) malloc(sizeof(int));
	nlist[0][0] = list[0];
	nlist[1] = NULL;
	if (sz) *sz = 1;
	return nlist;
    }
    olist = permutations(list+1,size-1,&osize);
    //osize = 0; while (olist[osize]) osize++;
    lpos = osize * size; lpos++;//set max list size
    nlist = (int**) malloc(sizeof(int*) * lpos);
    lpos = 0;

    for (i = 0; i < osize; i++) { // for each item on the old list
	for (j = 0; j < size; j++) {// for each position in the new list el
	    nlist[lpos] = (int*) malloc(sizeof(int) * size);
	    for (k = 0; k < j; k++) // copy elements before position
		nlist[lpos][k] = olist[i][k];
	    nlist[lpos][j] = list[0];
	    for (k = j+1; k <= size-1; k++)
		nlist[lpos][k] = olist[i][k-1];
	    lpos++;
	}// j loop
    }//i loop
    nlist[lpos] = NULL;
    for (i = 0; i < osize; i++)
	free(olist[i]);
    free(olist);
    if (sz) *sz = lpos;
    return nlist;
}

/**
 * @brief Determine all possible permutations (orderings) for a list of
 * things which contain only n items.
 *
 * permutations_nthings computes the list of possible permutations of
 * some list, limited to those permutations which contain only n
 * items. Ordering of elements in the returned list is not guarenteed.
 *
 * The number of possible permutations grows very quickly, and for any
 * significant list this function is likely to run out of memory and
 * crash. Since the function itself is recursive, overflowing the
 * stack is also a real possiability. This routine is not
 * particularlly efficent. Those with a serious need for this
 * functionality should go read Knuth, Art of Programming, vol 3.
 *
 * @param list A list of integers
 * @param num_el Size of the integer list
 * @param num The number of things in each permutation
 * @param sz[out] The number of permutations returned by the function. If
 * set to NULL then this information is not returned.
 * @return A list of permutation lists. Each permutation list is of
 * size size and is NOT null terminated. The list of permutations
 * itself is NULL terminated.
 **/

int** permutations_nthings(int* list, int num_el, int num, int* sz)
{
  int *map;
  int** pmap;
  int** comb;
  int** perm;
  int i, j, k;
  int psz, csz, nsz;

  if (num_el <= num) return permutations(list,num,sz);
 
  map = (int*) malloc(sizeof(int) * num);
  for (i = 0; i < num; i++) map[i] = i;
  pmap = permutations(map,num,&psz);
  free(map);

  comb = combinations(list,num_el,num,&csz);
  nsz = csz * psz;
  
  perm = (int**) malloc(sizeof(int*) * (nsz + 1));
  perm[nsz] = NULL;

  for (i = 0; i < csz; i++)
    for (j = 0; j < psz; j++) {
      perm[i * psz + j] = (int*) malloc(sizeof(int) * num);
      for (k = 0; k < num; k++)
	perm[i * psz + j][k] = comb[i][pmap[j][k]];
    } 

  free(pmap);
  free(comb);
  if (sz) *sz = nsz;
  return perm;
}

