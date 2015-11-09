#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define WINDOWSZ   512
#define BUFSZ      (3 * WINDOWSZ)

struct triple{
  int offset;
  int length;
  char c;
};

struct triple buffer[BUFSZ];
char          outp[WINDOWSZ];


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
  
  ssize_t buff_sz = read(fd_input, buffer, BUFSZ);

  int i, j = 0;
  struct triple t; 
  for(i = 0; i * sizeof(struct triple) < buff_sz; i++){
    t = buffer[i];
    int k; char c;
    for(k = 0; k < t.length; k++){
      c = outp[(j - t.offset + k) % WINDOWSZ];
      putchar(c);
      outp[(j + k) % WINDOWSZ] = c; 
    }
    j += t.length; 
    c = t.c;
    putchar(c);
    outp[j % WINDOWSZ] = c;
    j++;
  }

    

    
  

  
  return 0; 
}
