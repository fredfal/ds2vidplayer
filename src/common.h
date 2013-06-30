#ifndef _COMMON_H_
#define _COMMON_H_

#define INPUT_BUFFER_CHUNK_SIZE 32768
#define INPUT_BUFFER_CHUNK_NUMBER 32 // number of INPUT_BUFFER_CHUNK_SIZE

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
        int     used_bytes;
        int    frame_decoded;
} DecodeResult;

#endif

