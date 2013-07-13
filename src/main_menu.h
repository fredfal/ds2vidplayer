#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "menu.h"

MENU_OPTION_TYPE main_menu_options[9];

void main_menu_draw (PLAYER_CONTEXT * player_context);

void
main_menu_key (gui_action_type gui_action, PLAYER_CONTEXT * player_context);

#endif
