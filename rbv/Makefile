CC := g++
CFLAGS :=
DOPT := -O3
OPT := -g

SRCFILES := rbv.c

all: lib/rbv.o

lib/rbv.o: rbv.c
	$(CC) rbv.c -o lib/rbv.o -c $(CFLAGS) $(OPT)

test: rbv.test

rbv.test: rbv_test.c lib/rbv.o
	$(CC) rbv.c rbv_test.c -o rbv.test $(CFLAGS) $(OPT)

.PHONY: clean
clean:
	rm -f rbv_test rbv.o lib/rbv.o

