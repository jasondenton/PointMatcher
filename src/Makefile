CC=gcc 
CXX=g++
CFLAGS+=-D_HAS_LIBJPEG_ -Wall 
CXXFLAGS=$(CLFAGS)
STATICLIB=ar -r
LDFLAGS+=-lm -ljpeg 

#May need extra path information to find libjpeg
#Example: path to homebrew install libjpeg on Mac OS Arm Machinces
LDFLAGS+=-L/opt/homebrew/opt/jpeg/lib
CPPFLAGS+=-I/opt/homebrew/opt/jpeg/include

# Flags for MacOS
LDFLAGS+= -framework CoreServices
# Flags for everything else
#LDLAGS+= -lpthread

# Debug Setup
CFLAGS+=-g
LDFLAGS+=-g

# Optimized Build
# CFLAGS+=-Os -O3

OBJS=combinations.o jadimg.o num_proc.o qt_heuristic.o dict.o \
jadutil.o pmproblem.o pntset.o ransac.o dist_cord.o keyfeat.o pnteval.o \
projective.o similarity.o expr_sup.o lsearch.o qsort_2t.o solvps8.o pntmatch.o

all: pntmatcher markpnts

libpntmatch.a : $(OBJS)
	$(STATICLIB) $@ $^

pntmatcher: libpntmatch.a

markpnts: libpntmatch.a

clean:
	rm -f *.o
	rm -rf *.dSYM
	rm -f *.gcda
	rm -f *.gcno
	rm -f *.a
	rm -f pntmatcher
	rm -f markpnts
