#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <ds2_malloc.h>
#include <ds2io.h>

#include "video.h"
#include "jz4740_specific.h"

#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )

/*****************************************************************************
 *               Global vars in module and constants
 ****************************************************************************/

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192

static int XDIM = 0;
static int YDIM = 0;
static int CSP = XVID_CSP_RGB555;
static int BPP = 2;
static int ARG_THREADS = 1;

static void *dec_handle = NULL;

/*****************************************************************************
 *              output functions
 ****************************************************************************/
unsigned char *
video_resize_frame (unsigned char *out_buffer, int width, int height)
{
  /* Check if old buffer is smaller */
  if (XDIM * YDIM < width * height)
    {
      /* Copy new witdh and new height from the vol structure */
      XDIM = width;
      YDIM = height;

      printf ("New size seems to be %dx%d\n", XDIM, YDIM);

      /* Free old output buffer */
      if (out_buffer != NULL)
	{
	  printf ("Freeing allocated XViD buffer memory\n");
	  free (out_buffer);
	}

      /* Allocate the new buffer */
      out_buffer = (unsigned char *) malloc (XDIM * YDIM * 4);
      if (out_buffer == NULL)
	{
	  printf ("Unable to allocate new buffer\n");
	  return (unsigned char *) NULL;
	}

      printf ("Resized frame buffer to %dx%d\n", XDIM, YDIM);
      return out_buffer;
    }

  return out_buffer;
}


int
video_display_frame (unsigned char *image)
{
  //unsigned char *test_ipu_buffer;
  //test_ipu_buffer = (unsigned char*)malloc(XDIM*YDIM*4);
  //frameconversion_jz4740_ipu(image, test_ipu_buffer, XDIM, YDIM);
  //ds2_flipScreen(DOWN_SCREEN, 0);
  //return 0;

  unsigned short *buff;
  unsigned short *dst;
  unsigned short *end_buff;

  dst = up_screen_addr;
  buff = image;
  end_buff = buff + SCREEN_WIDTH * SCREEN_HEIGHT;

  do
    {
      *dst++ =
	(*buff & 0x7c00) >> 10 | (*buff & 0x03E0) | (*buff & 0x001F) << 10;
      buff++;
    }
  while (buff < end_buff);

  ds2_flipScreen (UP_SCREEN, 0);
  return 0;
}

/*****************************************************************************
 * Routines for decoding: init decoder, use, and stop decoder
 ****************************************************************************/

/* init decoder before first run */
int
dec_init (int debug_level)
{
  int ret;

  xvid_gbl_init_t xvid_gbl_init;
  xvid_dec_create_t xvid_dec_create;
  xvid_gbl_info_t xvid_gbl_info;

  /* Reset the structure with zeros */
  memset (&xvid_gbl_init, 0, sizeof (xvid_gbl_init_t));
  memset (&xvid_dec_create, 0, sizeof (xvid_dec_create_t));
  memset (&xvid_gbl_info, 0, sizeof (xvid_gbl_info));

	/*------------------------------------------------------------------------
	 * Xvid core initialization
	 *----------------------------------------------------------------------*/

  xvid_gbl_info.version = XVID_VERSION;
  xvid_global (NULL, XVID_GBL_INFO, &xvid_gbl_info, NULL);

  if (xvid_gbl_info.build != NULL)
    {
      printf ("xvidcore build version: %s\n", xvid_gbl_info.build);
    }
  printf ("Bitstream version: %d.%d.%d\n",
	  XVID_VERSION_MAJOR (xvid_gbl_info.actual_version),
	  XVID_VERSION_MINOR (xvid_gbl_info.actual_version),
	  XVID_VERSION_PATCH (xvid_gbl_info.actual_version));
  printf ("\n");

  /* Version */
  xvid_gbl_init.version = XVID_VERSION;
  xvid_gbl_init.debug = debug_level;

  xvid_global (NULL, 0, &xvid_gbl_init, NULL);

	/*------------------------------------------------------------------------
	 * Xvid decoder initialization
	 *----------------------------------------------------------------------*/

  /* Version */
  xvid_dec_create.version = XVID_VERSION;

  /*
   * Image dimensions -- set to 0, xvidcore will resize when ever it is
   * needed
   */
  xvid_dec_create.width = 0;
  xvid_dec_create.height = 0;

  xvid_dec_create.num_threads = ARG_THREADS;

  ret = xvid_decore (NULL, XVID_DEC_CREATE, &xvid_dec_create, NULL);

  dec_handle = xvid_dec_create.handle;

  return (ret);
}

/* decode one frame  */
int
dec_main (unsigned char *istream,
	  unsigned char *ostream,
	  int istream_size, xvid_dec_stats_t * xvid_dec_stats)
{

  int ret;
  xvid_dec_frame_t xvid_dec_frame;

  /* Reset all structures */
  memset (&xvid_dec_frame, 0, sizeof (xvid_dec_frame_t));
  memset (xvid_dec_stats, 0, sizeof (xvid_dec_stats_t));

  /* Set version */
  xvid_dec_frame.version = XVID_VERSION;
  xvid_dec_stats->version = XVID_VERSION;

  /* Input stream */
  xvid_dec_frame.bitstream = istream;
  xvid_dec_frame.length = istream_size;

  /* Output frame structure */
  xvid_dec_frame.output.plane[0] = ostream;
  xvid_dec_frame.output.stride[0] = XDIM * BPP;
  xvid_dec_frame.output.csp = CSP;

  ret =
    xvid_decore (dec_handle, XVID_DEC_DECODE, &xvid_dec_frame,
		 xvid_dec_stats);

  /* return number of bytes used */
  return (ret);
}

/* close decoder to release resources */
int
dec_stop ()
{
  int ret;

  ret = xvid_decore (dec_handle, XVID_DEC_DESTROY, NULL, NULL);

  return (ret);
}
