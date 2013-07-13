#ifndef _VIDEO_H_
#define _VIDEO_H_

#include "xvid.h"

int dec_init (int debug_level);
int dec_main (unsigned char *istream,
	      unsigned char *ostream,
	      int istream_size, xvid_dec_stats_t * xvid_dec_stats);
int dec_stop ();
int video_display_frame (unsigned char *image);
unsigned char *video_resize_frame (unsigned char *out_buffer, int width,
				   int height);

#endif
