# Makefile at repository root
CC=clang
CFLAGS=-O3 -std=c99 -Wall -Wextra
LDFLAGS_PTHREAD=-lpthread

all: sortseq sortpar

sortseq: sequential/main_seq.c
	$(CC) $(CFLAGS) sequential/main_seq.c -o sortseq

sortpar: parallel/main_par.c
	$(CC) $(CFLAGS) parallel/main_par.c -o sortpar $(LDFLAGS_PTHREAD)

clean:
	rm -f sortseq sortpar