CC := g++
CFLAGS :=
DOPT := -O3
OPT := -g

SRCFILES := ravl.cpp

all: lib/ravl.o

lib/ravl.o: ravl.cpp
	$(CC) ravl.cpp -o lib/ravl.o -c $(CFLAGS) $(OPT)

tests: ravl_test.cpp ravl.cpp ravl.hpp lib/ravl.o
	$(CC) ravl.cpp ravl_test.cpp -o ravl_test $(CFLAGS) $(OPT)

.PHONY: clean
clean:
	rm -f ravl ravl.o lib/ravl.o ravl_test

