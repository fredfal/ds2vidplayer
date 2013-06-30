#ifndef _SOUND_H_
#define _SOUND_H_

#include "common.h"

void mad_decode(unsigned char *istream,
                 signed short *ostream,
                 int istream_size,
		 DecodeResult* decode_result);
int mad_init();

void sound_play_frame(signed short* ostream)
int sound_completion();

#endif
