
CFLAGS = -O3 -Wall 
all: lzw.c decompress.c
	gcc $(CFLAGS) lzw.c -o lzw
	gcc $(CFLAGS) decompress.c -o decompress
	g++ $(CFLAGS) compressseq.cc -o compress
