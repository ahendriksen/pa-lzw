
CFLAGS = -O3 -Wall
all: lzw decompress compress

lzw: lzw.c lzw.h
	gcc $(CFLAGS) lzw.c -o lzw

decompress: decompress.c lzw.h
	gcc $(CFLAGS) decompress.c -o decompress

compress: compressseq.cc
	g++ $(CFLAGS) compressseq.cc -o compress
