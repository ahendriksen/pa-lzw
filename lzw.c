#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define WINDOWSZ   512
#define BUFSZ	   (3 * WINDOWSZ)


struct triple{
  int offset;
  int length;
  char c;
};

int compress_buffer(char *buffer, int length, int start, int end, 
		    struct triple *output);


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

  int		i, j;		// i is index into buffer, j into outp
  struct triple best_t;		// contains the best triple for each i
  int		l, offset;	// contains running variables
  int		max_offset;

  int window_offset = 0,
      max_i         = 2 * WINDOWSZ;

  i = 0, j = 0; 
  for(window_offset = 0; ; window_offset += WINDOWSZ){
    for(; i < max_i; i += best_t.length + 1){
      best_t.offset = 0;
      best_t.length = 0;
      best_t.c      = buffer[i];

      // Look for common substrings
      max_offset = (i + 1 < WINDOWSZ) ? i + 1 : WINDOWSZ; 
      for(offset = 0; offset < max_offset; offset++){
	// The following must hold
	// - i - o + l < i
	// - i + l < buff_len
	// - There must be a char left at the end of the
	//   buffer to put in the triple.
	for(l = 0; l < offset && i + l < buff_len - 1; l++){
	  if(buffer[i + l] != buffer[i - offset + l]){
	    break;
	  }
	}
	if(best_t.length < l){
	  best_t.offset = offset;
	  best_t.length = l;
	  best_t.c      = buffer[i + l];
	}
      }

      // best_t now contains the best triple for this index i
      printf("(%d,%d,%c)\n", best_t.offset, best_t.length, best_t.c);
      // store it
      outp[j] = best_t;
      
      j++;
    }
    // We've run through 2/3 of the buffer
    memcpy(buffer	    , buffer + WINDOWSZ,     WINDOWSZ);
    memcpy(buffer + WINDOWSZ, buffer + 2 * WINDOWSZ, WINDOWSZ); 
    int read_sz = read(fd_input, buffer + 2 * WINDOWSZ, WINDOWSZ); 

    // write the triples
    write(fd_output, outp, j * sizeof(outp[0]));
    j = 0; 

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
		    struct triple *output)
{

  int		i, j;	// i is index into buffer, j into outp
  struct triple best_t;	// contains the best triple for each i
  int		l, o;	// contains length, offset running variables
  int		max_offset;

  i = start; 
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
      for(l = 0; l <= o && i + l < length; l++){
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
    printf("(%d,%d,%c)\n", best_t.offset, best_t.length, best_t.c);
    // store it
    outp[j] = best_t;
      
    j++;
  }
  return i; 
}
