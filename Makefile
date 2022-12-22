CC=gcc
CFLAGS = -Og

.PHONY: default test extmem

default: test extmem
	$(CC) $(CFLAGS) ./out/test.o ./out/extmem.o -o test

test: test.c
	$(CC) $(CFLAGS) -c test.c -o ./out/test.o

extmem: extmem.c
	$(CC) $(CFLAGS) -c extmem.c -o ./out/extmem.o

clean:
	rm ./out/*.o
	rm test