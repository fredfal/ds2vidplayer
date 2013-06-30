#include <limits.h>
#include <stdio.h>
#include <ds2io.h>
#include <ds2_malloc.h>

#include "mad.h"
#include "common.h"

struct mad_stream	Stream;
struct mad_frame	Frame;
struct mad_synth	Synth;

unsigned char* mad_audio_buffer;
int mad_audio_buffer_used_size;

#define MAD_AUDIO_BUFFER_SIZE 4032

struct audio_stats {
  unsigned long clipped_samples;
  mad_fixed_t peak_clipping;
  mad_fixed_t peak_sample;
};

struct audio_dither {
  mad_fixed_t error[3];
  mad_fixed_t random;
};

/*
 * NAME:	prng()
 * DESCRIPTION:	32-bit pseudo-random number generator
 */
static unsigned long prng(unsigned long state)
{
  return (state * 0x0019660dL + 0x3c6ef35fL) & 0xffffffffL;
}

/*
 * NAME:	audio_linear_dither()
 * DESCRIPTION:	generic linear sample quantize and dither routine
 */
static signed long audio_linear_dither(unsigned int bits, mad_fixed_t sample,
				struct audio_dither *dither,
				struct audio_stats *stats)
{
  unsigned int scalebits = 0;
  mad_fixed_t output = 0, mask = 0, random = 0;

  enum {
    MIN = -MAD_F_ONE,
    MAX =  MAD_F_ONE - 1
  };

  /* noise shape */
  sample += dither->error[0] - dither->error[1] + dither->error[2];

  dither->error[2] = dither->error[1];
  dither->error[1] = dither->error[0] / 2;

  /* bias */
  output = sample + (1L << (MAD_F_FRACBITS + 1 - bits - 1));

  scalebits = MAD_F_FRACBITS + 1 - bits;
  mask = (1L << scalebits) - 1;

  /* dither */
  random  = prng(dither->random);
  output += (random & mask) - (dither->random & mask);

  dither->random = random;

  /* clip */
  if (output >= stats->peak_sample) {
    if (output > MAX) {
      ++stats->clipped_samples;
      if (output - MAX > stats->peak_clipping)
	stats->peak_clipping = output - MAX;

      output = MAX;

      if (sample > MAX)
	sample = MAX;
    }
    stats->peak_sample = output;
  }
  else if (output < -stats->peak_sample) {
    if (output < MIN) {
      ++stats->clipped_samples;
      if (MIN - output > stats->peak_clipping)
	stats->peak_clipping = MIN - output;

      output = MIN;

      if (sample < MIN)
	sample = MIN;
    }
    stats->peak_sample = -output;
  }

  /* quantize */
  output &= ~mask;

  /* error feedback */
  dither->error[0] = sample - output;

  /* scale */
  return output >> scalebits;
}

/****************************************************************************
 * Print human readable informations about an audio MPEG frame.				*
 ****************************************************************************/
static int PrintFrameInfo(struct mad_header *Header)
{
	const char	*Layer,
				*Mode,
				*Emphasis;

	/* Convert the layer number to it's printed representation. */
	switch(Header->layer)
	{
		case MAD_LAYER_I:
			Layer="I";
			break;
		case MAD_LAYER_II:
			Layer="II";
			break;
		case MAD_LAYER_III:
			Layer="III";
			break;
		default:
			Layer="(unexpected layer value)";
			break;
	}

	/* Convert the audio mode to it's printed representation. */
	switch(Header->mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
			Mode="single channel";
			break;
		case MAD_MODE_DUAL_CHANNEL:
			Mode="dual channel";
			break;
		case MAD_MODE_JOINT_STEREO:
			Mode="joint (MS/intensity) stereo";
			break;
		case MAD_MODE_STEREO:
			Mode="normal LR stereo";
			break;
		default:
			Mode="(unexpected mode value)";
			break;
	}

	/* Convert the emphasis to it's printed representation. Note that
	 * the MAD_EMPHASIS_RESERVED enumeration value appread in libmad
	 * version 0.15.0b.
	 */
	switch(Header->emphasis)
	{
		case MAD_EMPHASIS_NONE:
			Emphasis="no";
			break;
		case MAD_EMPHASIS_50_15_US:
			Emphasis="50/15 us";
			break;
		case MAD_EMPHASIS_CCITT_J_17:
			Emphasis="CCITT J.17";
			break;
#if (MAD_VERSION_MAJOR>=1) || \
	((MAD_VERSION_MAJOR==0) && (MAD_VERSION_MINOR>=15))
		case MAD_EMPHASIS_RESERVED:
			Emphasis="reserved(!)";
			break;
#endif
		default:
			Emphasis="(unexpected emphasis value)";
			break;
	}

	printf("%lu kb/s audio mpeg layer %s stream %s crc, "
			"%s with %s emphasis at %d Hz sample rate\n",
			Header->bitrate,Layer,
			Header->flags&MAD_FLAG_PROTECTION?"with":"without",
			Mode,Emphasis,Header->samplerate);
}

static const char *MadErrorString(const struct mad_stream *Stream)
{
	switch(Stream->error)
	{
		/* Generic unrecoverable errors. */
		case MAD_ERROR_BUFLEN:
			return("input buffer too small (or EOF)");
		case MAD_ERROR_BUFPTR:
			return("invalid (null) buffer pointer");
		case MAD_ERROR_NOMEM:
			return("not enough memory");

		/* Frame header related unrecoverable errors. */
		case MAD_ERROR_LOSTSYNC:
			return("lost synchronization");
		case MAD_ERROR_BADLAYER:
			return("reserved header layer value");
		case MAD_ERROR_BADBITRATE:
			return("forbidden bitrate value");
		case MAD_ERROR_BADSAMPLERATE:
			return("reserved sample frequency value");
		case MAD_ERROR_BADEMPHASIS:
			return("reserved emphasis value");

		/* Recoverable errors */
		case MAD_ERROR_BADCRC:
			return("CRC check failed");
		case MAD_ERROR_BADBITALLOC:
			return("forbidden bit allocation value");
		case MAD_ERROR_BADSCALEFACTOR:
			return("bad scalefactor index");
		case MAD_ERROR_BADFRAMELEN:
			return("bad frame length");
		case MAD_ERROR_BADBIGVALUES:
			return("bad big_values count");
		case MAD_ERROR_BADBLOCKTYPE:
			return("reserved block_type");
		case MAD_ERROR_BADSCFSI:
			return("bad scalefactor selection info");
		case MAD_ERROR_BADDATAPTR:
			return("bad main_data_begin pointer");
		case MAD_ERROR_BADPART3LEN:
			return("bad audio data length");
		case MAD_ERROR_BADHUFFTABLE:
			return("bad Huffman table select");
		case MAD_ERROR_BADHUFFDATA:
			return("Huffman data overrun");
		case MAD_ERROR_BADSTEREO:
			return("incompatible block_type for JS");

		/* Unknown error. This swich may be out of sync with libmad's
		 * defined error codes.
		 */
		default:
			return("Unknown error code");
	}
}

/****************************************************************************
 * Converts a sample from mad's fixed point number format to a signed		*
 * short (16 bits).															*
 ****************************************************************************/
static signed short MadFixedToSshort(mad_fixed_t Fixed)
{
	/* A fixed point number is formed of the following bit pattern:
	 *
	 * SWWWFFFFFFFFFFFFFFFFFFFFFFFFFFFF
	 * MSB                          LSB
	 * S ==> Sign (0 is positive, 1 is negative)
	 * W ==> Whole part bits
	 * F ==> Fractional part bits
	 *
	 * This pattern contains MAD_F_FRACBITS fractional bits, one
	 * should alway use this macro when working on the bits of a fixed
	 * point number. It is not guaranteed to be constant over the
	 * different platforms supported by libmad.
	 *
	 * The signed short value is formed, after clipping, by the least
	 * significant whole part bit, followed by the 15 most significant
	 * fractional part bits. Warning: this is a quick and dirty way to
	 * compute the 16-bit number, madplay includes much better
	 * algorithms.
	 */

	/* Clipping */
	if(Fixed>=MAD_F_ONE)
		return(SHRT_MAX);
	if(Fixed<=-MAD_F_ONE)
		return(-SHRT_MAX);

	/* Conversion. */
	Fixed=Fixed>>(MAD_F_FRACBITS-15);
	return((signed short)Fixed);
}

int mad_init()
{
	mad_audio_buffer = (unsigned char*)malloc(MAD_AUDIO_BUFFER_SIZE);
	memset(mad_audio_buffer, 0, MAD_AUDIO_BUFFER_SIZE);
	mad_audio_buffer_used_size = 0;

	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);

	return 0;
}

struct audio_stats Audio_stats;
struct audio_dither Audio_dither;

void mad_decode(unsigned char *istream,
                 signed short *ostream,
                 int istream_size,
		 DecodeResult *result)
{
	int i;
	int remaining;
	
	result->frame_decoded=0;

	memcpy(mad_audio_buffer+mad_audio_buffer_used_size, istream, istream_size);
	mad_audio_buffer_used_size += istream_size;
	memset(mad_audio_buffer+mad_audio_buffer_used_size,0,MAD_BUFFER_GUARD);

	mad_stream_buffer(&Stream, mad_audio_buffer, mad_audio_buffer_used_size);

	if (mad_frame_decode(&Frame,&Stream))
	{
		if(Stream.error==MAD_ERROR_BUFLEN) 
		{
			// we need to buffer more data
			result->used_bytes = istream_size;
			return;
		}

		if(MAD_RECOVERABLE(Stream.error))
		{
			printf("recoverable frame level error (%s)\n",
				MadErrorString(&Stream));
			mad_audio_buffer_used_size = 0;
			result->used_bytes = istream_size;
			return;
		}
		else
		{
			printf("unrecoverable frame level error (%s).\n",
				MadErrorString(&Stream));
			return;
		}
	}

	result->frame_decoded=1;
	result->used_bytes=mad_audio_buffer_used_size;

	// the frame was decoded
	if(Stream.next_frame!=NULL)
	{
		remaining=Stream.bufend-Stream.next_frame; // Remaining i.e not used by mad
		memmove(mad_audio_buffer, Stream.next_frame, remaining);
		mad_audio_buffer_used_size = remaining;
	}
	else
	{
		mad_audio_buffer_used_size = 0;
	}

	mad_synth_frame(&Synth,&Frame);

	int numOut = 0;
	
	mad_fixed_t const *left_ch   = &(Synth.pcm.samples)[ 0 ];
	mad_fixed_t const *right_ch  = &(Synth.pcm.samples)[ 1 ];
	
	memset(&Audio_stats,0,sizeof(Audio_stats));
	memset(&Audio_dither,0,sizeof(Audio_dither));

	switch(Synth.pcm.channels)
	{
		default://default is stereo
			for ( i = 0; i < Synth.pcm.length; i++ )
			{
				ostream[numOut++] = audio_linear_dither(16, (*left_ch++),&Audio_dither,&Audio_stats);
				ostream[numOut++] = audio_linear_dither(16, (*right_ch++),&Audio_dither,&Audio_stats);
			}
		break;
		case 1:
			for ( i = 0; i < Synth.pcm.length; i++ )
			{
				signed short Sample = audio_linear_dither(16, (*left_ch++),&Audio_dither,&Audio_stats);
				ostream[numOut++] = Sample;
				ostream[numOut++] = Sample;
			}
			
		break;
	}
}

#ifndef AUDIO_BUFFER_COUNT
#define AUDIO_BUFFER_COUNT 4
#endif
void sound_play_frame(signed short* ostream)
{
	int i;
	signed short *audio_buffer_addr;

        while(ds2_checkAudiobuff() >= AUDIO_BUFFER_COUNT)
        {
                mdelay(1);
        }

	i = 0;
	do
	{
		audio_buffer_addr = (signed short *) ds2_getAudiobuff();
	}
	while ((audio_buffer_addr == NULL) && (i++ < 1000));

	if (audio_buffer_addr != NULL)
	{
		signed short *dst0 = audio_buffer_addr;
		signed short *dst1 = audio_buffer_addr + Synth.pcm.length;

		i=0;

		while(i++ < Synth.pcm.length){
			switch(Synth.pcm.channels){
				case 2://stereo
					*dst0++ = *ostream++;
					*dst1++ = *ostream++;
				break;
				case 1://mono
					*dst0++ = *ostream++;
				break;
			}
		}
	}	

	ds2_updateAudio();
}

int sound_completion() {
        return ds2_checkAudiobuff() >= AUDIO_BUFFER_COUNT;
}
