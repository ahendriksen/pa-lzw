#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lzw.h" 

// # of triples to read in at once. 
#define BUFFER_SIZE  512

void decompress(int fd_input); 


int main(int argc, char * argv[])
{
  if(argc != 2){
    printf("Provide file name. \n");
    exit(1);
  }

  int fd_input  = open(argv[1], O_RDONLY);
  if(fd_input == -1){
    printf("Input file could not be opened. \n");
    exit(1);
  }


  return 0; 
}

void decompress(int fd_input)
{ 
  static struct triple buffer[BUFFER_SIZE];
  static char          lookback[MAX_LOOKBACK];
  
  ssize_t buff_sz;
  int i, j = 0;
  int k; char c;
  while((buff_sz = read(fd_input, buffer, BUFFER_SIZE * sizeof(buffer[0]))) > 0){ 
    struct triple t; 
    for(i = 0; i * sizeof(struct triple) < buff_sz; i++){
      t = buffer[i];
      // decode prefix
      for(k = 0; k < t.length; k++){
	c = lookback[(j - t.offset + k) % MAX_LOOKBACK];
	putchar(c);
	lookback[(j + k) % MAX_LOOKBACK] = c; 
      }
      j += t.length; 
      putchar(t.c);
      lookback[j++ % MAX_LOOKBACK] = t.c;
    }
  }
}
