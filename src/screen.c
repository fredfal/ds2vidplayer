#include <stdio.h>

#include "ffReader_config.h"
#include "ds2io.h"
#include "image.h"
#include "filetypes_config.h"
#include "ds2_malloc.h"
#include "fs.h"
#include "fs_api.h"

int load_image_file(int screen_id, const char* archive_filename, const char* filename)
{
        t_fs_imagefile_spec_entry *imagefile_spec;
        t_fs_archivefile_spec_entry *archivefile_spec;
	image_data_s *image_data;
        FILE *stream;

	image_data = (image_data_s*)malloc(sizeof(image_data_s));

        if (archive_filename == NULL)
	{
                imagefile_spec = get_imagefile_spec(filename);
	        stream = fat_fopen(filename, "r");
                image_data = imagefile_spec->load_image(stream, SCREEN_WIDTH, SCREEN_HEIGHT, &fat_fread, &fat_fseek, image_data);
                fat_fclose(stream);

		load_image_data(0, image_data);
                free(image_data->data);
	} 
        else
	{
		archivefile_spec = get_archivefile_spec(archive_filename);
                imagefile_spec = get_imagefile_spec(filename);

		stream = archivefile_spec->fopen_function(archive_filename, filename, "r");
		image_data = imagefile_spec->load_image(stream, SCREEN_WIDTH, SCREEN_HEIGHT, archivefile_spec->fread_function, archivefile_spec->fseek_function, image_data);
		archivefile_spec->fclose_function(stream);

		load_image_data(screen_id, image_data);
		free(image_data->data);
	}

	free(image_data);
};

int load_image_data(int screen_id, image_data_s *image_data)
{
        unsigned int x, y;
        unsigned short *dst = NULL;
        unsigned int width = SCREEN_WIDTH;
        unsigned int height = SCREEN_HEIGHT;
	unsigned int w = (SCREEN_WIDTH - image_data->width) / 2;
        unsigned int h = (SCREEN_HEIGHT - image_data->height) / 2;

	//ds2_clearScreen(screen_id, WHITE_COLOR);
        //ds2_flipScreen(screen_id, 0);

	// convert image pixels according to image depth
	if(image_data->depth == DEPTH_8BIT)
        {
                unsigned short *pt;

                for(y= 0; y< height; y++)
                {
                        dst = (unsigned short*)down_screen_addr + (y+h)*SCREEN_WIDTH +w;
                        pt = (unsigned short*)image_data->data + y*SCREEN_WIDTH*2;
                        for(x= 0; x< width; x++)
                        {
                                *dst++= RGB16_15(pt);
                                pt += 1;
                        }
                }
        }
        else if(image_data->depth == DEPTH_24BIT) 
        {
                unsigned char *buff;

                for(y= 0; y< height; y++)
                {
                        dst = (unsigned short*)down_screen_addr + (y+h)*SCREEN_WIDTH +w;
                        buff = image_data->data + y*SCREEN_WIDTH*4;
                        for(x= 0; x< width; x++)
                        {
                                *dst++= RGB24_15(buff);
                                buff += 3;
                        }
                }
        }

        ds2_flipScreen(screen_id, 0);
}
