/**
 * @file num_proc.c
 * @author Jason Denton
 * @version 0.95
 * @date May, 2009
 *
 * Copyright Jason Denton, 2009. This source code is made
 * available under the terms of the new BSD license. See the file
 * license.txt for the applicable terms.
 **/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#ifdef __sgi__
#include <sys/sysmp.h>
#endif

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

//use touch foo.h; cpp -dM foo.h; rm foo.h to get a list of platform
//specific defines


/**
 * @brief Determine the number of processors available in the system.
 *
 * This function returns the number of processors available in the
 * system. If the environmental variable NUMBER_OF_PROCESSORS is set
 * then that value is returned instead of whatever value the system is
 * reporting. Note that in most cases multi-threaded CPUs are reported
 * as providng multiple processors.
 * @return Number of available processors.
 **/

int number_of_processors(void)
{
#define __DEFAULT__NP

  char* evar;
#ifdef __FreeBSD__
  int numcpus = 0;
  int buflen = 4;
  int mib[2] = {CTL_HW,HW_NCPU};
#endif

#ifdef __APPLE__
  int numcpus = 0;
  size_t buflen;


#endif


  evar = getenv("NUMBER_OF_PROCESSORS");
  if (evar) return atoi(evar);

#ifdef __APPLE__
#undef __DEFAULT__NP
  buflen = sizeof(int);
  sysctlbyname("hw.activecpu", &numcpus, &buflen, NULL, 0);
  return numcpus;
#endif

#ifdef __FreeBSD__
#undef __DEFAULT__NP
  sysctl(mib, 2, &numcpus, &buflen, NULL, 0);
  return numcpus;
#endif

#ifdef __sun__
#undef __DEFAULT__NP
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif

#ifdef __sgi__
#undef __DEFAULT__NP
  return sysmp(MP_NPROCS);
#endif

#ifdef __linux__
#undef __DEFAULT__NP 
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif

#ifdef __DEFAULT__NP
  return 1;  
#endif
}
