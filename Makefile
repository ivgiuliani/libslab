CC=gcc
CFLAGS=-O2 -ggdb -DCASSERT
SONAME=libslab.so

TESTS = \
	test/test-simple \
	test/test-intcache \
	test/test-multislab \
	test/test-largeobj \
	test/test-constructor \
	test/test-giant \
	test/test-benchmark

.PHONY: clean

all:
	$(CC) $(CFLAGS) -c -fPIC src/slab.c -o src/slab.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,$(SONAME) -o $(SONAME) -Wall src/slab.o

clean: clean-tests
	-rm -f libslab.so
	-rm -f src/*.o

clean-tests:
	-rm -f test/test-*

test/test-%: test/%.c
	$(CC) -g -Isrc/ -L. -Wall $< -lslab -o $@
	@env LD_LIBRARY_PATH=. $@

test: all clean-tests $(TESTS)

