#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LOOKBACK 512
#define WINDOWSZ     512
#define BUFSZ	     (WINDOWSZ + 2 * MAX_LOOKBACK)


struct triple{
  int  offset;
  int  length;
  char c;
};

int compress_buffer(char *buffer, int length, int start, int end, 
		    struct triple *output, int fd_out);

char	      buffer[BUFSZ];
struct triple outp[BUFSZ];


int main(int argc, char * argv[])
{
  if(argc != 2){
    printf("Provide file name. \n");
    exit(1);
  }

  int fd_input  = open(argv[1], O_RDONLY);
  int fd_output = open("output.bin", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU);
  if(fd_input == -1){
    printf("Input file could not be opened. \n");
    exit(1);
  } else if (fd_output == -1){
    printf("Output file could not be opened. \n");
  }

  
  ssize_t buff_len = read(fd_input, buffer, BUFSZ * sizeof(buffer[0]));

  int i             = 0; 
  int window_offset = 0,
      max_i         = 2 * WINDOWSZ;

  for(window_offset = 0; ; window_offset += WINDOWSZ){
    // compress buffer: 
    i = compress_buffer(buffer, buff_len, i, max_i, outp, fd_output);

    // copy the last two-thirds of the buffer to the front
    // we use memcpy twice in order to avoid overlapping memory regions
    memcpy(buffer,	      buffer + WINDOWSZ,     WINDOWSZ);
    memcpy(buffer + WINDOWSZ, buffer + 2 * WINDOWSZ, WINDOWSZ); 
    int read_sz = read(fd_input, buffer + 2 * WINDOWSZ, WINDOWSZ); 

    // determine how much more work we have to do
    if(read_sz == WINDOWSZ){	    // buffer filled
      max_i = 2 * WINDOWSZ;         // buff_len unchanged 
    } else if (0 < read_sz &&
	       read_sz < WINDOWSZ){ // buffer partially filled
      max_i = 2 * WINDOWSZ + read_sz;
      buff_len = max_i; 
    } else if(i < buff_len){	    // end of file, but not done
      buff_len -= WINDOWSZ;         // buffer has been shifted left
      max_i = buff_len;		    // finish everything
    } else {			    // end of file, done
      break;
    }

    i = i % (2 * WINDOWSZ);
  }

  return 0; 
}


/* compress_buffer

   arguments: 
   - buffer: input buffer;
   - length: length of the input buffer;
   - start: start compression at index start;
   - end: stop compressing before end 
     (strictly speaking: the last triple should not start with the char at end)
   - output: buffer for output triples. Should have length equal to buffer. 

   returns: 
   an int with the last index used for compression. 
 */ 
int compress_buffer(char *buffer, int length, int start, int end, 
		    struct triple *output, int fd_out)
{

  int		i, j;	// i is index into buffer, j into output
  struct triple best_t;	// contains the best triple for each i
  int		l, o;	// contains length, offset running variables
  int		max_offset;

  i = start; 
  j = 0;
  for(; i < end; i += best_t.length + 1){
    best_t.offset = 0;
    best_t.length = 0;
    best_t.c      = buffer[i];

    // Look for common substrings
    max_offset = (i + 1 < WINDOWSZ) ? i + 1 : WINDOWSZ; // TODO: not sure this is correct
    for(o = 0; o < max_offset; o++){
      // The following must hold
      // - i - o + l < i
      // - i + l < buff_len
      // - There must be a char left at the end of the
      //   buffer to put in the triple.
      for(l = 0; l < o && i + l < length; l++){
	if(buffer[i + l] != buffer[i - o + l]){
	  break; // l is now the length of the common substring
	}
      }
      if(best_t.length < l){
	best_t.offset = o;
	best_t.length = l;
	best_t.c      = buffer[i + l];
      }
    }

    // best_t now contains the best triple for this index i
    // printf("(%d,%d,%c)\n", best_t.offset, best_t.length, best_t.c);
    // store it
    output[j] = best_t;
      
    j++;
  }

  // write the triples
  if(write(fd_out, output, j * sizeof(output[0])) < 0){ 
    fprintf(stderr, "Could not write to output file\n"); 
    exit(3); 
  }
  return i; 
}
