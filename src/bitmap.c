#include "stdio.h"
#include "bitmap.h"
#include "fs_api.h"
#include "image.h"
#include "ds2_malloc.h"
#include "ds2io.h"

#ifndef u32
	#define u32 unsigned int
#endif

#ifndef s32
	#define s32 int
#endif

#ifndef u16
	#define u16 unsigned short
#endif

extern image_data_s *BMP_read(void* fp, int max_width, int max_height, size_t (*fread_function)(void *ptr, size_t size, size_t nmemb, void *stream), int (*fseek_function)(void *stream, long offset, int whence), image_data_s *image_data)
{
	BMPHEADER bmp_header;
	int	flag;
	u32 bytepixel;
	u32	x, y, sx, sy, m, len;
	unsigned char *dest;
	s32	fpos;
	unsigned char st[54];
        int	image_original_width, image_original_height;

	if(fp == NULL)
        {
		image_data->error_code = BMP_ERR_OPENFAILURE;
                return image_data;
        }

	flag= fread_function(st, sizeof(st), 1, fp);
        if(!flag)
        {
                image_data->error_code = BMP_ERR_FORMATE;
                return image_data;
        }

	bmp_header.bfType= *((u16*)st);
	bmp_header.bfSize= *((u16*)(st+2)) | *((u16*)(st+4));
	bmp_header.bfReserved0= *((u16*)(st+6));
	bmp_header.bfReserved1= *((u16*)(st+8));
	bmp_header.bfImgoffst= *((u16*)(st+10)) | *((u16*)(st+12));
	bmp_header.bfImghead.imHeadsize= *((u16*)(st+14)) | *((u16*)(st+16));
	bmp_header.bfImghead.imBitmapW= *((u16*)(st+18)) | *((u16*)(st+20));
	bmp_header.bfImghead.imBitmapH= *((u16*)(st+22)) | *((u16*)(st+24));
	bmp_header.bfImghead.imPlanes= *((u16*)(st+26));
	bmp_header.bfImghead.imBitpixel= *((u16*)(st+28));
	bmp_header.bfImghead.imCompess= *((u16*)(st+30)) | *((u16*)(st+32));
	bmp_header.bfImghead.imImgsize= *((u16*)(st+34)) | *((u16*)(st+36));
	bmp_header.bfImghead.imHres= *((u16*)(st+38)) | *((u16*)(st+40));
	bmp_header.bfImghead.imVres= *((u16*)(st+42)) | *((u16*)(st+44));
	bmp_header.bfImghead.imColnum= *((u16*)(st+46)) | *((u16*)(st+48));
	bmp_header.bfImghead.imImcolnum= *((u16*)(st+50)) | *((u16*)(st+52));
   
	if(bmp_header.bfType != 0x4D42)	//"BM"
	{
		image_data->error_code = BMP_ERR_FORMATE;
                return image_data;
	}

	if(bmp_header.bfImghead.imCompess != BI_RGB && 
		bmp_header.bfImghead.imCompess != BI_BITFIELDS)
	{
		image_data->error_code = BMP_ERR_NEED_GO_ON;		//This funciton now not support...
                return image_data;
	}

	bytepixel= bmp_header.bfImghead.imBitpixel >> 3;
	if(bytepixel < 2 || bytepixel > 3)		//byte per pixel >= 2
	{
		image_data->error_code = BMP_ERR_NEED_GO_ON;		//This funciton now not support...
                return image_data;
	}

        if (bytepixel == 2)
		image_data->depth = DEPTH_8BIT;
        if (bytepixel == 3)
		image_data->depth = DEPTH_24BIT;

	image_original_width = bmp_header.bfImghead.imBitmapW;
	image_original_height = bmp_header.bfImghead.imBitmapH;

	image_data->width = max_width < image_original_width ? max_width : image_original_width;
        image_data->height = max_height < image_original_height ? max_height : image_original_height;


        image_data->data = (char*)malloc(image_data->height*image_data->width*4);

        if (image_data->data == NULL)
        {
		printf("Malloc failed\n");
	}

	//BMP scan from down to up
	fpos= (s32)bmp_header.bfImgoffst;
	dest= (unsigned char*)image_data->data + (image_data->height - 1) * image_data->width * 4;

	for(m= 0; m < image_data->height; m++) {
		fseek_function(fp, fpos, SEEK_SET);
		fread_function(dest, 1, image_data->width*bytepixel, fp);
		fpos += ((image_original_width * bytepixel+3)>>2)<<2;
		dest -= image_data->width * 4;
	}

	return image_data;
}
