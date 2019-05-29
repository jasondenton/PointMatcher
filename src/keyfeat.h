/**
 * @file keyfeat.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicabe terms.
 **/

#ifndef KEYFEAT_H
#define KEYFEAT_H

#include "pmproblem.h"

#ifdef __cplusplus
extern "C" {
#endif

  Match* key_features(PntMatchProblem, int, long, size_t*)

#ifdef __cplusplus
}
#endif

#endif
