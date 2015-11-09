
CFLAGSS = -g -O2 -Wall 
all: lzw.c decompress.c
	gcc $(CFLAGS) lzw.c -o lzw
	gcc $(CFLAGS) decompress.c -o decompress
