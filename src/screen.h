#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "image.h"

int load_image_file(int screen_id, const char* archive_filename, const char* filename);
int load_image_data(int screen_id, image_data_s *image_data);

#endif
