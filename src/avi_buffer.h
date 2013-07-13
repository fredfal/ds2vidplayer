
#ifndef _AVI_BUFFER_H
#define _AVI_BUFFER_H
  
#include <ds2_types.h>
#include "common.h"
  
#ifndef bool
#define bool int
#endif /*  */
  
#ifndef false
#define false 0
#endif /*  */
  
#ifndef true
#define true 1
#endif /*  */
  
#include "avi_headers.h"
  
#define AVI_DATATYPE_IGNORE 0
#define AVI_DATATYPE_AUDIO 1
#define AVI_DATATYPE_VIDEO 2
  
#define AVI_CHUNK_SIZE 32768	// 32 KiB
#define AVI_MIN_CHUNKS 2
  
#define MAX_KEYFRAME_SEARCH_ATTEMPTS 100
#define AVI_SEEK_OFFSET_FACTOR (1<<24)
#define AVI_START_CHUNK_TOLERANCE 20
  typedef struct
{
  int dataType;
   u8 * currentPosition;
   int amountLeft;
 } AviChunkData;
 typedef struct
{
  u32 audioFourcc;
  u32 videoFourcc;
  FILE * aviFile;
  int fileLen;
   int moviSize;
   int idx1Offset;
   int idx1Size;
   int indexOffset;
   u8 * bufferStart;
   int moviRemain;
   u8 * writePointer;
   u8 * readPointer;
   int amountLeft;
   AviChunkHeader chunkHeader;
   AviChunkData chunkData;
   int nextChunkOffset;
 } AviBuffer;
 extern AviBuffer aviBuffer;
 bool aviBufferInit (FILE * aviFile, int *frameWidth, int *frameHeight,
		       int *frameRate, int *numFrames);
void aviBufferFree (AviBuffer * aBuffer);
 int aviBufferLoadChunk (AviBuffer * aBuffer);
void aviBufferRefill (AviBuffer * aBuffer);
 AviChunkData aviGetCurrentChunk (AviBuffer * aBuffer);
void aviUsedAmount (AviBuffer * aBuffer, int amount);

//bool aviSeekChunk (int chunk);
//bool aviSeekOffset (int offset);
  
#endif // _AVI_BUFFER_H
