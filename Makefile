CC=gcc
CFLAGS=-O2 -ggdb

.PHONY: clean

all:
	$(CC) $(CFLAGS) -c -fPIC src/slab.c -o src/slab.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libslab.so -o libslab.so -Wall src/slab.o

clean:
	rm -f libslab.so
	rm -f src/*.o
	rm -f test/simple
	rm -f test/multislab

test: all
	$(CC) -Isrc/ -L. -Wall test/simple.c -lslab -o test/simple
	env LD_LIBRARY_PATH=. ./test/simple
