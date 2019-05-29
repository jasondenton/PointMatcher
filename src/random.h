/**
 * @file random.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

/* Copyright Jason Denton 2003 */
#include <stdlib.h>
#include <time.h>

/*
#define set_random_seed(X) srand48(X)
#define randomize() srand48(time(NULL))
#define randint(X) (lrand48() % (X))
#define random lrand48()
#define random01 drand48()
*/

#define set_random_seed(X) srandom(X)
#define randomize() srandom(time(NULL))
#define randint(X) (random() % (X))
//#define random random()
#define random01 1.0 / (double) (random() + 1)
