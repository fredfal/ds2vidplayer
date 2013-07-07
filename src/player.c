#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <fs_api.h>
#include <ds2_malloc.h>
#include <ds2io.h>
#include <ds2_timer.h>

#include "common.h"
#include "video.h"
#include "avi_buffer.h"

/*****************************************************************************
 *               Global vars in module and constants
 ****************************************************************************/
#define MIN_USEFUL_BYTES 1

/*****************************************************************************
 *        Main program
 ****************************************************************************/

int play_file(char *filename)
{
	int frameWidth;
	int frameHeight;
	int frameRate;
	int numFrames;

	frameWidth=-1;
	frameHeight=-1;
	frameRate=-1;
	numFrames=-1;

	/* output buffers */
	unsigned char *video_out_buffer = NULL;
	signed short *audio_out_buffer = NULL;

	static xvid_dec_stats_t xvid_dec_stats;
	
	int status;
	int debug_level = 0;
  
	FILE *fd_input_file;

/*****************************************************************************
 * Openning file
 ****************************************************************************/

	fd_input_file = fopen(filename, "rb");
	if (fd_input_file == NULL) {
		printf("Error opening input file %s\n", filename);
		return(-1);
	}

/*****************************************************************************
 *        Xvid initialisation
 ****************************************************************************/

	status = dec_init(debug_level);
	if (status) {
		printf("Decore INIT problem, return value %d\n", status);
		goto release_all;
	}

/*****************************************************************************
 *        Mad initialisation
 ****************************************************************************/

	status = mad_init();
        if (status) {
                printf("Mad INIT problem, return value %d\n", status);
                goto release_all;
        }

/*****************************************************************************
 *        Input buffer initialisation
 ****************************************************************************/

	/* Fil the buffer */
	aviBufferInit(fd_input_file, &frameWidth, &frameHeight, &frameRate, &numFrames);

/*****************************************************************************
 *        Output buffer initialisation
 ****************************************************************************/

	//video_out_buffer=(unsigned char*)malloc(4*1024*1024);
	audio_out_buffer=(signed short*)malloc(2*1152*sizeof(signed short));

/*****************************************************************************
 *	                         main loop
 ****************************************************************************/
	int j=0;
	AviChunkData currentChunkData;
	DecodeResult decode_result;

	do {
   	        unsigned int key;
        	struct key_buf input;
                key = getInput(&input);

		if(key)
            	{
                	if(input.key & KEY_TOUCH)
	                	printf("x,y = (%d, %d)\n", input.x, input.y);
	                else if(input.key & KEY_LID)
        	            printf("key_lid\n");
                	else if(input.key & KEY_UP)
	                    printf("key_up\n");
        	        else if(input.key & KEY_DOWN)
                	    printf("key_down\n");
	                else if(input.key & KEY_LEFT)
        	            printf("key_left\n");
                	else if(input.key & KEY_RIGHT)
	                    printf("key_right\n");
        	        else if(input.key & KEY_L)
                	    printf("key_l\n");
	                else if(input.key & KEY_R)
        	            printf("key_r\n");
                	else if(input.key & KEY_A)
	                    printf("key_a\n");
        	        else if(input.key & KEY_B)
                	    printf("key_b\n");
	                else if(input.key & KEY_X)
        	            printf("key_x\n");
                	else if(input.key & KEY_Y)
	                    printf("key_y\n");
        	        else if(input.key & KEY_START)
                	    printf("key_start\n");
	                else if(input.key & KEY_SELECT)
        	            printf("key_select\n");
            	}

		/*
		 * if the buffer is half empty or there are no more bytes in it
		 * then fill it.
		 */
		if (aviBuffer.amountLeft < INPUT_BUFFER_CHUNK_NUMBER*INPUT_BUFFER_CHUNK_SIZE) {
			aviBufferRefill(&aviBuffer);
		}

		decode_result.used_bytes = 0;
		decode_result.frame_decoded = 0;
		currentChunkData = aviGetCurrentChunk(&aviBuffer);

		switch (currentChunkData.dataType) {
			case AVI_DATATYPE_AUDIO:
		 		mad_decode((unsigned char*)currentChunkData.currentPosition, audio_out_buffer, currentChunkData.amountLeft, &decode_result);

				if (decode_result.frame_decoded == 1)
				{
					j++;
					sound_play_frame(audio_out_buffer);
				}

				aviUsedAmount(&aviBuffer, decode_result.used_bytes);
				break;
			case AVI_DATATYPE_VIDEO:
				decode_result.used_bytes = dec_main((unsigned char*)currentChunkData.currentPosition, video_out_buffer, currentChunkData.amountLeft, &xvid_dec_stats);
				if (xvid_dec_stats.type == XVID_TYPE_VOL) {
                	        	video_out_buffer = video_resize_frame(video_out_buffer, xvid_dec_stats.data.vol.width, xvid_dec_stats.data.vol.height);
                	              	if (video_out_buffer == NULL) {
                	                	printf("Cannot resize / allocate XViD output buffer\n");
                	              	}
                	      	}

				while(sound_completion()) { mdelay(1); }

				if (xvid_dec_stats.type > 0) {
					if(video_display_frame(video_out_buffer)) {
						printf("Error writing decoded frame\n");
					}
				}
	
				aviUsedAmount(&aviBuffer, decode_result.used_bytes);
				break;
			default:
				aviUsedAmount(&aviBuffer, currentChunkData.amountLeft);
		}
	} while (!feof(fd_input_file) || aviBuffer.amountLeft > 0);

/*****************************************************************************
 *      Xvid PART  Stop
 ****************************************************************************/

 release_all:
 	status = dec_stop();
	if (status)    
		printf("decore RELEASE problem return value %d\n", status);

 free_all_memory:
	free(audio_out_buffer);
	free(video_out_buffer);

	return(0);
}
