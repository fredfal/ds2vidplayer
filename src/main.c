//main.c

#include <stdio.h>
#include <ds2io.h>

#include "screen.h"
#include "ds2io.h"
#include "player.h"

void main(int argc, char* argv[])
{
	//ds2_setCPUclocklevel(10);
	printf_clock();

	player_main("fat:/test.avi");
}


