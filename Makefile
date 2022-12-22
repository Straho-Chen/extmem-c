CC=gcc
CFLAGS = -Og

.PHONY: default test temp extmem

default: test extmem
	$(CC) $(CFLAGS) ./out/test.o ./out/extmem.o -o test
# default: temp extmem
# 	$(CC) $(CFLAGS) ./out/temp.o ./out/extmem.o -o test

test: test.c
	$(CC) $(CFLAGS) -c test.c -o ./out/test.o

temp: temp.c
	$(CC) $(CFLAGS) -c temp.c -o ./out/temp.o

extmem: extmem.c
	$(CC) $(CFLAGS) -c extmem.c -o ./out/extmem.o

clean:
	rm ./out/*.o
	rm test