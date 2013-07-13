/*
*
* This file is adapted from Tuna-viDS by Chishm source code
*
*/  
#include <stdint.h>
#include <string.h>
#include <ds2_malloc.h>
#include <fs_api.h>
  
#include "common.h"
#include "avi_buffer.h"
  AviBuffer aviBuffer =
{
0};

 int
aviBufferFreeSize (AviBuffer * aBuffer)
{
  if ((unsigned int) aBuffer->writePointer ==
       (unsigned int) aBuffer->readPointer && aBuffer->amountLeft == 0)
    {
      return INPUT_BUFFER_CHUNK_NUMBER * INPUT_BUFFER_CHUNK_SIZE;
    }
   if ((unsigned int) aBuffer->writePointer >
	 (unsigned int) aBuffer->readPointer)
    {
      return INPUT_BUFFER_CHUNK_NUMBER * INPUT_BUFFER_CHUNK_SIZE -
	(unsigned int) aBuffer->writePointer +
	(unsigned int) aBuffer->readPointer;
    }
  else
    {
      if ((unsigned int) aBuffer->writePointer <=
	   (unsigned int) aBuffer->readPointer)
	{
	  return (unsigned int) aBuffer->readPointer -
	    (unsigned int) aBuffer->writePointer;
	}
} }  int

aviBufferLoadChunk (AviBuffer * aBuffer)
{
  if (!aBuffer->bufferStart)
    {
      return 0;
    }
   int len;
   if (aviBufferFreeSize (aBuffer) < 2 * INPUT_BUFFER_CHUNK_SIZE)
    
    {
      
	// Buffer full
	return 0;
    }
   len =
    fread (aBuffer->writePointer, 1, INPUT_BUFFER_CHUNK_SIZE,
	   aBuffer->aviFile);
   if (len <= 0)
    {
      return 0;
    }
   if (len < INPUT_BUFFER_CHUNK_SIZE && !feof (aBuffer->aviFile))
    {
      printf ("Error reading file !!!\n");
      return -1;
    }
   aBuffer->writePointer += len;
  
    // Wrap the write pointer back to the beginning of the buffer if needed
    if (aBuffer->writePointer >
	aBuffer->bufferStart + (INPUT_BUFFER_CHUNK_NUMBER -
				1) * INPUT_BUFFER_CHUNK_SIZE)
    {
      
	// We copy the last chunk at the first chunk position
	memcpy (aBuffer->bufferStart,
		aBuffer->bufferStart + (INPUT_BUFFER_CHUNK_NUMBER -
					1) * INPUT_BUFFER_CHUNK_SIZE, len);
      
	// We set the pointer to the second chunk position
	aBuffer->writePointer =
	aBuffer->bufferStart + INPUT_BUFFER_CHUNK_SIZE;
    }
   aBuffer->amountLeft += len;
   return len;
}

 void
aviBufferRefill (AviBuffer * aBuffer)
{
  while (aviBufferLoadChunk (aBuffer) > 0);
}

 AviChunkData aviGetCurrentChunk (AviBuffer * aBuffer)
{
  return aBuffer->chunkData;
}

 void
getStreamNextDataChunk (AviBuffer * aBuffer)
{
  
  do
    {
      aBuffer->readPointer += aBuffer->nextChunkOffset;
      aBuffer->readPointer = (u8 *) (((u32) aBuffer->readPointer + 1) & ~1);	// Word align 
      aBuffer->amountLeft -= aBuffer->nextChunkOffset;
       
	//aviBuffer.moviRemain -= aviBuffer.nextChunkOffset; // TO CHECK!!!!
	
	// Wrap the pointer to the start of the buffer if we are close to the enf of buffer (first chunk in the buffer = last one)
	if (aBuffer->readPointer >=
	    aBuffer->bufferStart + (INPUT_BUFFER_CHUNK_NUMBER -
				    1) * INPUT_BUFFER_CHUNK_SIZE)
	{
	  aBuffer->readPointer =
	    aBuffer->readPointer - (INPUT_BUFFER_CHUNK_NUMBER -
				    1) * INPUT_BUFFER_CHUNK_SIZE;
	}
       
	// Ensure there is enough data in the buffer
	while (aBuffer->amountLeft < AVI_MIN_CHUNKS * INPUT_BUFFER_CHUNK_SIZE
	       && aviBufferLoadChunk (aBuffer) > 0);
       memcpy (&(aBuffer->chunkHeader), aBuffer->readPointer,
		 sizeof (AviChunkHeader));
       if (aBuffer->chunkHeader.fourcc == LIST_ID)
	{			// List
	  aBuffer->nextChunkOffset = sizeof (AviListHeader);
	}
      else
	{			// Chunk
	  if (aBuffer->chunkHeader.fourcc == aBuffer->videoFourcc)
	    {
	      aBuffer->chunkData.dataType = AVI_DATATYPE_VIDEO;
	    }
	  else
	    {
	      if (aBuffer->chunkHeader.fourcc == aBuffer->audioFourcc)
		{
		  aBuffer->chunkData.dataType = AVI_DATATYPE_AUDIO;
		}
	      else
		{
		  aBuffer->chunkData.dataType = AVI_DATATYPE_IGNORE;
		}
	    }
	   aBuffer->chunkData.currentPosition =
	    aBuffer->readPointer + sizeof (AviChunkHeader);
	  aBuffer->chunkData.amountLeft = aBuffer->chunkHeader.size;
	  aBuffer->nextChunkOffset =
	    aBuffer->chunkHeader.size + sizeof (AviChunkHeader);
	}
    }
  while (
	 ((aBuffer->chunkHeader.fourcc != aBuffer->videoFourcc
	   && aBuffer->chunkHeader.fourcc !=
	   aBuffer->audioFourcc)  ||aBuffer->chunkData.amountLeft ==
	  0) /* skip chunks that don't contain any data */  
&&aBuffer->amountLeft > 0);
}

 void
aviUsedAmount (AviBuffer * aBuffer, int amount)
{
  aBuffer->chunkData.currentPosition += amount;
  aBuffer->chunkData.amountLeft -= amount;
   if (aBuffer->chunkData.amountLeft <= 0 || amount == 0)
    {
      getStreamNextDataChunk (aBuffer);
       if (aBuffer->chunkHeader.fourcc != aBuffer->videoFourcc
	     && aBuffer->chunkHeader.fourcc != aBuffer->audioFourcc)
	{
	  aBuffer->chunkData.amountLeft = 0;
	}
    }
}

 
/*
bool aviSeekChunk (int chunk) {
	AviIndexEntry aviIndexEntry;
	int chunkOffset;
	bool foundKeyframe;
	int i;

	if (!aviBuffer.idx1Offset || !aviBuffer.aviFile) {
		return false;
	}
	
	if (chunk * sizeof(AviIndexEntry) >= aviBuffer.idx1Size) {
		return false;
	}
	
	// Reset the buffer status
	aviBuffer.videoReadPointer = aviBuffer.videoDataChunk.currentPosition = aviBuffer.bufferStart;
	aviBuffer.videoAmountLeft = aviBuffer.audioAmountLeft = 0;
	aviBuffer.audioReadPointer = aviBuffer.writePointer = aviBuffer.bufferStart;

	// Get index entry for chunk
	fseek (aviBuffer.aviFile, aviBuffer.idx1Offset + chunk * sizeof(AviIndexEntry), SEEK_SET);
	// Try to find a key frame
	foundKeyframe = false;
	if (chunk > AVI_START_CHUNK_TOLERANCE) {
		for (i = 0; i < MAX_KEYFRAME_SEARCH_ATTEMPTS && !foundKeyframe; i++) {
			fread (&aviIndexEntry, sizeof(AviIndexEntry), 1, aviBuffer.aviFile);
			foundKeyframe = (aviIndexEntry.flags & AVIIF_KEYFRAME) && (aviIndexEntry.fourcc == CHUNK_video);
		}
	}
	if (!foundKeyframe) {
		// Just use what was found
		fseek (aviBuffer.aviFile, aviBuffer.idx1Offset + chunk * sizeof(AviIndexEntry), SEEK_SET);
		fread (&aviIndexEntry, sizeof(AviIndexEntry), 1, aviBuffer.aviFile);
	}
	// Calculate file offset
	chunkOffset = aviBuffer.indexOffset + aviIndexEntry.offset;
	// Jump to chunk and refill buffer
	fseek (aviBuffer.aviFile, chunkOffset, SEEK_SET);
	aviBuffer.moviRemain = aviBuffer.moviSize - (chunkOffset - aviMoviListStart + sizeof (AviListHeader));
	aviBufferRefill ();
	// Seek to start point for video
	aviBuffer.videoChunkHeader.size = 0;
	aviBuffer.videoDataChunk.amountLeft = 0;
	aviVideoUsedAmount (0);

	return true;
}

bool aviSeekOffset (int offset) {
	int64_t chunk;
	
	chunk = (((int64_t)aviBuffer.idx1Size/sizeof(AviIndexEntry)) * (int64_t)offset) / (int64_t)AVI_SEEK_OFFSET_FACTOR;

	return aviSeekChunk ((int)chunk);
}*/ 
  bool aviBufferInit (FILE * aviFile, int *frameWidth, int *frameHeight,
		       int *frameRate, int *numFrames)
{
  RiffHeader riffHeader;
  AviListHeader aviListHeader;
  AviMainHeader aviMainHeader;
  AviStreamHeader aviStreamHeader;
  AviChunkHeader aviChunkHeader;
  AviIndexEntry aviIndexEntry;
   int aviMoviListStart;
  int nextListStart;
   aviBuffer.aviFile = aviFile;
  fat_fseek (aviBuffer.aviFile, 0, SEEK_END);
  aviBuffer.fileLen = ftell (aviBuffer.aviFile);
  fseek (aviBuffer.aviFile, 0, SEEK_SET);
   if (aviBuffer.fileLen <= 0)
    {
      return false;
    }
   
    // Read in AVI file info
    // RIFF AVI header
    fread (&riffHeader, sizeof (RiffHeader), 1, aviBuffer.aviFile);
  if (riffHeader.riffID != RIFF_ID || riffHeader.fourcc != RIFF_AVI_FOURCC)
    return false;
   
    // Initial AVI header
    fread (&aviListHeader, sizeof (AviListHeader), 1, aviBuffer.aviFile);
  if (aviListHeader.listID != LIST_ID || aviListHeader.fourcc != LIST_hdrl)
    return false;
  
    // Calculate where the actual data starts within the file
    aviMoviListStart =
    ftell (aviBuffer.aviFile) + ((aviListHeader.size + 1) & ~1) -
    (sizeof (AviListHeader) - 8);
   
    // Main AVI header, gives info on frame size
    fread (&aviMainHeader, sizeof (AviMainHeader), 1, aviBuffer.aviFile);
  if (aviMainHeader.fourcc != CHUNK_avih)
    return false;
  fseek (aviBuffer.aviFile,
	  ((aviMainHeader.size + 1) & ~1) - (sizeof (AviMainHeader) - 8),
	  SEEK_CUR);
  if (frameWidth)
    *frameWidth = aviMainHeader.width;
  if (frameHeight)
    *frameHeight = aviMainHeader.height;
   
    // First stream list -- must be video stream!
    fread (&aviListHeader, sizeof (AviListHeader), 1, aviBuffer.aviFile);
  if (aviListHeader.listID != LIST_ID || aviListHeader.fourcc != LIST_strl)
    return false;
  nextListStart =
    ftell (aviBuffer.aviFile) + ((aviListHeader.size + 1) & ~1) -
    (sizeof (AviListHeader) - 8);
  fread (&aviChunkHeader, sizeof (AviChunkHeader), 1, aviBuffer.aviFile);
  if (aviChunkHeader.fourcc != CHUNK_strh)
    return false;
  fread (&aviStreamHeader, sizeof (AviStreamHeader), 1, aviBuffer.aviFile);
  if (aviStreamHeader.fourcc != STREAM_vids)
    return false;
  
    // Calculate frame rate in centihertz
    if (frameRate)
    *frameRate = aviStreamHeader.rate * 100 / aviStreamHeader.scale;
  if (numFrames)
    *numFrames = aviStreamHeader.length;
   
    // Second stream list -- must be audio stream!
    fseek (aviBuffer.aviFile, nextListStart, SEEK_SET);
  fread (&aviListHeader, sizeof (AviListHeader), 1, aviBuffer.aviFile);
  if (aviListHeader.listID != LIST_ID || aviListHeader.fourcc != LIST_strl)
    return false;
  nextListStart =
    ftell (aviBuffer.aviFile) + ((aviListHeader.size + 1) & ~1) -
    (sizeof (AviListHeader) - 8);
  fread (&aviChunkHeader, sizeof (AviChunkHeader), 1, aviBuffer.aviFile);
  if (aviChunkHeader.fourcc != CHUNK_strh)
    return false;
  fread (&aviStreamHeader, sizeof (AviStreamHeader), 1, aviBuffer.aviFile);
  if (aviStreamHeader.fourcc != STREAM_auds)
    return false;
   
    // Get information about movie data
    do
    {
      fseek (aviBuffer.aviFile, aviMoviListStart, SEEK_SET);
      fread (&aviListHeader, sizeof (AviListHeader), 1, aviBuffer.aviFile);
      if (aviListHeader.fourcc != LIST_movi
	   || aviListHeader.listID != LIST_ID)
	{
	  aviMoviListStart =
	    ftell (aviBuffer.aviFile) + ((aviListHeader.size + 1) & ~1) -
	    (sizeof (AviListHeader) - 8);
	}
    }
  while (aviListHeader.fourcc != LIST_movi && !feof (aviBuffer.aviFile) && 
	 (aviListHeader.listID == LIST_ID
	  || aviListHeader.listID == JUNK_ID));
   if (aviListHeader.fourcc != LIST_movi)
    return false;
   aviBuffer.moviSize =
    ((aviListHeader.size + 1) & ~1) - (sizeof (AviListHeader) - 8);
   
    // Find index
    fseek (aviBuffer.aviFile, aviBuffer.moviSize, SEEK_CUR);
  fread (&aviChunkHeader, sizeof (AviChunkHeader), 1, aviBuffer.aviFile);
  if (feof (aviBuffer.aviFile))
    {
      aviBuffer.idx1Offset = 0;
    }
  else if (aviChunkHeader.fourcc != CHUNK_idx1)
    {
      aviBuffer.idx1Offset = 0;
    }
  else
    {
      aviBuffer.idx1Offset = ftell (aviBuffer.aviFile);
      aviBuffer.idx1Size =
	((aviChunkHeader.size + 1) & ~1) - (sizeof (AviListHeader) - 8);
      fread (&aviIndexEntry, sizeof (AviIndexEntry), 1, aviBuffer.aviFile);
      aviBuffer.indexOffset =
	aviMoviListStart + sizeof (AviListHeader) - aviIndexEntry.offset;
    }
   
    // Allocate memory for buffer
    aviBuffer.bufferStart =
    (u8 *) malloc (INPUT_BUFFER_CHUNK_NUMBER * INPUT_BUFFER_CHUNK_SIZE);
  if (!aviBuffer.bufferStart)
    return false;
  aviBuffer.readPointer = aviBuffer.bufferStart;
  aviBuffer.writePointer = aviBuffer.bufferStart;
   
    // Jump back to movie stream, ready to play
    fseek (aviBuffer.aviFile, aviMoviListStart + sizeof (AviListHeader),
	   SEEK_SET);
   
    // Fill AVI buffer so we have some data to work with
    aviBuffer.moviRemain = aviBuffer.moviSize;
  aviBufferRefill (&aviBuffer);
   
    // Seek to start point for video
    aviBuffer.videoFourcc = CHUNK_video;
  aviBuffer.audioFourcc = CHUNK_audio;
  aviBuffer.nextChunkOffset = 0;
  aviBuffer.chunkHeader.size = 0;
  aviBuffer.chunkData.amountLeft = 0;
  aviUsedAmount (&aviBuffer, 0);
   return true;
}

 void
aviBufferFree (AviBuffer * aBuffer)
{
  if (!aBuffer->bufferStart)
    {
      return;
    }
  free (aBuffer->bufferStart);
  aBuffer->bufferStart = NULL;
}

 
