#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// MAX_LOOKBACK is the maximum number of chars that the lzw algorithm
// is allowed to look back. 
// WINDOWSZ is the buffer window that is loaded into memory at once. 
// BUFSZ takes into account the margins that have to be loaded as well. 

#define K            1024
#define M            (K*K)

#define MAX_LOOKBACK (512)
#define WINDOWSZ     M
#define BUFSZ	     (WINDOWSZ + 2 * MAX_LOOKBACK)

struct triple{
  int offset;
  int length;
  char c;
};

void compress_file(FILE* file_input, FILE* file_output);
int compress_buffer(char *buffer, int length, int start, int end, 
		    struct triple *output, FILE* file_out);

static char	      buffer[BUFSZ];
static struct triple  outp[BUFSZ];


inline int min(int a, int b)
{
  return (a < b) ? a : b; 
}

int main(int argc, char * argv[])
{
  if(argc != 2){
    printf("Provide file name. \n");
    exit(1);
  }

  FILE *fd_input  = fopen(argv[1], "r");
  FILE *fd_output = fopen("output.bin", "w");
  if(fd_input == NULL){
    printf("Input file could not be opened. \n");
    exit(1);
  } else if (fd_output == NULL){
    printf("Output file could not be opened. \n");
  }

  compress_file(fd_input, fd_output); 

  return 0; 
}

/* compress_file
   
   arguments: 
   - file_input: file to compress; 
   - file_output: file to write the compressed byte stream.
*/ 
void compress_file(FILE *file_input, FILE *file_output)
{
  int i     = 0; 
  int max_i = MAX_LOOKBACK + WINDOWSZ;

  size_t buff_len = fread(buffer, 1, BUFSZ, file_input);
  max_i = (max_i < buff_len) ? max_i : buff_len; 
  i = compress_buffer(buffer, buff_len, i, max_i, outp, file_output); 
  while(i < buff_len){ 
    // assume max_i <= i
    // move two blocks of lookback at the end to the front
    memmove(buffer, buffer + WINDOWSZ - MAX_LOOKBACK, 2 * MAX_LOOKBACK); 
    buff_len = fread(buffer + 2 * MAX_LOOKBACK, 1, WINDOWSZ, file_input); 
    buff_len += 2 * MAX_LOOKBACK; 

    i -= WINDOWSZ; // We just moved the buffer WINDOWSZ to the left
    max_i = min(MAX_LOOKBACK + WINDOWSZ, buff_len); 
    i = compress_buffer(buffer, buff_len, i, max_i, outp, file_output); 
  }
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
		    struct triple *output, FILE *file_out)
{
  int		i, j;	// i is index into buffer, j into output
  struct triple best_t;	// contains the best triple for each i
  int		l, o;	// contains length, offset running variables

  int max_length, max_offset;
  j = 0;
  for(i = start; i < end; i += best_t.length + 1){
    best_t.offset = 0;
    best_t.length = 0;
    best_t.c      = buffer[i];

    // for max_offset we must not reach 
    //   in front of the buffer
    //   outside of MAX_LOOKBACK
    max_offset = min(i, MAX_LOOKBACK); 
    for(o = max_offset; best_t.length < o; o--){
      // length cannot exceed offset. 
      // Also, the last triple must contain buffer[length - 1] as character. 
      max_length = min(o, length - i - 1); 
      for(l = 0; l < max_length; l++){
    	if(buffer[i + l] != buffer[i - o + l]){
    	  break;
    	}
      }
      if(best_t.length < l) {
    	best_t.offset = o;
    	best_t.length = l;
    	best_t.c      = buffer[i + l];
      }
    }

    // best_t now contains the best triple for this index i
    // printf("(%d,%d,%c)\n", best_t.offset, best_t.length, best_t.c);
    // store it
    /* lengths[best_t.length] ++; */
    output[j++] = best_t;
  }

  // write the triples
  if(fwrite(output, sizeof(output[0]), j, file_out) != j){
    fprintf(stderr, "Could not write to output file\n");
    exit(3);
  }
  return i; 
}
