CC=gcc
CFLAGS = -Og

.PHONY: default main extmem

default: main extmem
	$(CC) $(CFLAGS) ./out/main.o ./out/extmem.o -o main

main: main.c
	$(CC) $(CFLAGS) -c main.c -o ./out/main.o

extmem: extmem.c
	$(CC) $(CFLAGS) -c extmem.c -o ./out/extmem.o

clean:
	rm ./out/*.o
	rm main