include Make_Setup.inc

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
