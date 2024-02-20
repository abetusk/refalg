CC := g++
CFLAGS :=
DOPT := -O3
OPT := -g

SRCFILES := ravl.cpp

all: lib/ravl.o

lib/ravl.o: ravl.cpp
	$(CC) ravl.cpp -o lib/ravl.o -c $(CFLAGS) $(OPT)

tests: ravl_test.cpp ravl.cpp ravl.hpp lib/ravl.o
	$(CC) ravl.cpp ravl_test.cpp -o ravl.test $(CFLAGS) $(OPT)
	$(CC) ravl.cpp rlet_test.cpp -o rlet.test $(CFLAGS) $(OPT)

ravl.test: ravl_test.cpp ravl.cpp ravl.hpp lib/ravl.o
	$(CC) ravl.cpp ravl_test.cpp -o ravl.test $(CFLAGS) $(OPT)

rlet.test: ravl.cpp ravl.hpp lib/ravl.o rlet.hpp rlet_test.cpp
	$(CC) ravl.cpp rlet_test.cpp -o rlet.test $(CFLAGS) $(OPT)

.PHONY: clean
clean:
	rm -f ravl ravl.o lib/ravl.o ravl_test

