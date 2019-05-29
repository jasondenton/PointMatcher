/**
 * @file jadutil.h
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Texas Tech University, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#ifndef _JDMULTI_H_
#define _JDMULTI_H_

#include <stdio.h>
#include <unistd.h>
/**
 * @struct list_proc_obj
 * @brief An object describing a list to be processed, and how to
 * process it.
 *
 * The purpose of list_proc_obj is to encapsulate the information
 * needed to process a list, perhaps spreading the work across
 * multiple processors.  The fields of a list_proc_obj can be set
 * manuually, or by calling \link get_list_proc_obj. The actual
 * semantics of a list_proc_obj are provided by the \link process_list
 * routine.
 **/


typedef struct {
  void** list; ///< A list of pointers to the objects to be processed.
  unsigned long list_size; ///< The number of objects on the list.
  void* shared;
  /**< A pointer to a block of static data to be shared
   * between all threads processing the objects on the list. For
   * instance, a problem general problem description or parameter block.
  **/
  
  void* (*process_item)(void*,void*,void*);
  /**< The function used to process each item on the list. The
   * function provided will be called once for each item on the
   * list. The first parameter to this function is the pointer to the
   * shared block. The second parameter is a pointer to the per-thread
   * object for the thread which is processing this object. The final
   * pointer is the pointer to the object to be processed. The return
   * value of this function is placed on the list returned by
   * process_list, in the same position as the object that was
   * processed to produce it.
   **/
  
  void* (*allocate_scratch_space)(void*);
  /**< A function which sets up an per-thread object or
   * memory block. Unlike the shared object, each thread will get its own copy
   * of this object. If set to NULL no additional memory will be
   * allocated to each thread. 
   **/
	
  void (*free_scratch_space)(void*,void*);
  /**<
   * A function to clean up the per-thread resources allocated by
   * allocate_scratch_space. Not needed if allocate_scratch_space is set
   * to NULL. If allocate_scratch_space is not NULL but this field is,
   * then the standard free function will be called for each per-thread
   * object.
   **/
} list_proc_obj;


#ifdef  __cplusplus
extern "C" {
#endif

  int number_of_processors(void);
  
  void** process_list(list_proc_obj);
  list_proc_obj get_list_proc_obj(void**, unsigned long,void*,
				  void* (*t)(void*,void*,void*));
  void qsort_2t(void*, size_t, size_t, int (*cmpfunc)(const void*,const void*));
  
#ifdef  __cplusplus
}
#endif

#endif
