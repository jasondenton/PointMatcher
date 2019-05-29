# Contents of this file are not meant to be changed by the user.
# Instead, change the variables defined in config.inc,
# Or run the provided configure script to set the appropriate
# variables in the environment.

include config.inc

ifeq ($(CPU),PRESCOTT)
CFLAGS+=-mtune=prescott -march=prescott -msse3 -mfpmath=sse
endif

ifeq ($(CPU),PENTIUMPRO)
CFLAGS+=-mtune=pentiumpro -march=pentiumpro
endif

ifeq ($(CPU),G5)
CFLAGS+=-march=G5 -mtune=G5 -mpowerpc64 -mpowerpc-gpopt -faltivec
endif

ifeq ($(CPU),G4)
CFLAGS+=-march=G4 -mtune=G4 -faltivec
endif

#Debugging/Optimization
ifeq ($(DEBUG),YES)
CFLAGS+=-g -fprofile-arcs -ftest-coverage
LDFLAGS+=-g -fprofile-arcs -ftest-coverage
else
CFLAGS+=-Os -O3
endif

#Platform defines
ifeq ($(OPSYS),Darwin)
LDFLAGS+=-lm -ljpeg -framework CoreServices
endif

ifeq ($(OPSYS),FreeBSD)
LDFLAGS+=-lm -ljpeg -lpthread 
endif

ifeq ($(OPSYS),Linux)
LDFLAGS+=-lm -ljpeg -lpthread 
endif

#Compilier defines
CC=gcc 
CXX=g++
CFLAGS+=-D_HAS_LIBJPEG_ -Wall -I /usr/local/include 
CXXFLAGS=$(CLFAGS)
LDFLAGS+= -L /usr/local/lib
STATICLIB=ar -r
