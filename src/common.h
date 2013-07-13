#ifndef _COMMON_H_
#define _COMMON_H_

#include "ds2_types.h"

#define INPUT_BUFFER_CHUNK_SIZE 32768
#define INPUT_BUFFER_CHUNK_NUMBER 32	// number of INPUT_BUFFER_CHUNK_SIZE

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef bool
#define bool int
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 1
#endif

typedef struct
{
  int used_bytes;
  int frame_decoded;
} DecodeResult;

#endif
