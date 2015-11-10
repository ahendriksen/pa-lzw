#define K            1024
#define M            (K*K)


// MAX_LOOKBACK is the maximum number of chars that the lzw algorithm
// is allowed to look back. 
// WINDOWSZ is the buffer window that is loaded into memory at once. 
// BUFSZ takes into account the margins that have to be loaded as well. 

#define MAX_LOOKBACK (512)
#define WINDOWSZ     (4*K)
#define BUFSZ	     (WINDOWSZ + 2 * MAX_LOOKBACK)

struct triple{
  int offset;
  int length;
  char c;
};
