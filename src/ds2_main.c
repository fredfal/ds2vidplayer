//ds2_main.c

#include <stdio.h>
#include "console.h"
#include "fs_api.h"
#include "ds2io.h"
#include "jz4740_specific.h"

#define BLACK_COLOR		RGB15(0, 0, 0)
#define WHITE_COLOR		RGB15(31, 31, 31)

extern void main(int argc, char* argv[]);

void ds2_main(void)
{
	int err;
	int mxucr;

	//Initial video and audio and other input and output
	err = ds2io_init(1152);
	if (err) goto _failure;

	//Initial console for printf funciton
	//err = ConsoleInit(WHITE_COLOR, BLACK_COLOR, UP_SCREEN, 1);
	//if (err) goto _failure;

	//Initial file system
	err = fat_init();
	if (err) goto _failure;

	mxucr = enable_jz4740_mxu();
	enable_jz4740_ipu();

        ds2_setCPUclocklevel(13);
        printf_clock();

        gui_init();

	while (1) {
		menu();
	}

        play_file("fat:/test.avi");
_failure:
	printf("some error\n");
	while(1);
	ds2_plug_exit();
}

