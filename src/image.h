#ifndef _IMAGE_H_
#define _IMAGE_H_

typedef enum
{ DEPTH_8BIT, DEPTH_24BIT } image_depth_e;

typedef struct {
    image_depth_e depth;
    int width;
    int height;
    int error_code;
    char *data;
} image_data_s;

#define RGB24_15(pixel) ((((*pixel) & 0xF8) << 7) |\
                        (((*(pixel+1)) & 0xF8) << 2) |\
                        (((*(pixel+2)) & 0xF8)>>3))

#define RGB16_15(pixel) ((((*pixel)>>10) & 0x1F) |\
                                                (((*pixel) & 0x1F) << 10) |\
                                                ((*pixel) & 0x83E0))

#define BLACK_COLOR             RGB15(0, 0, 0)^M
#define WHITE_COLOR             RGB15(31, 31, 31)
#define GREEN_COLOR             RGB15(0, 31, 0)

#endif
