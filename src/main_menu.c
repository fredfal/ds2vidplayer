#include "message.h"
#include "draw.h"
#include "menu.h"
#include "gui.h"

MENU_OPTION_TYPE main_menu_options[] = {
  /*
   * 00 
   */ ACTION_OPTION (menu_play, NULL, NULL, 0),
  /*
   * 01 
   */ ACTION_OPTION (NULL, NULL, NULL, 1),
  /*
   * 02 
   */ ACTION_OPTION (NULL, NULL, NULL, 3),
  /*
   * 03 
   */ ACTION_OPTION (NULL, NULL, NULL, 4),
  /*
   * 04 
   */ ACTION_OPTION (NULL, NULL, NULL, 5),
  /*
   * 05 
   */ ACTION_OPTION (NULL, NULL, NULL, 6),
  /*
   * 06 
   */ ACTION_OPTION (NULL,
		     &msg[MSG_MAIN_MENU_INFO],
		     NULL,
		     7),
  /*
   * 07 
   */ ACTION_OPTION (menu_loadfile,
		     &msg[MSG_MAIN_MENU_LOAD],
		     NULL, 8),
  /*
   * 08 
   */ ACTION_OPTION (NULL,
		     &msg[MSG_MAIN_MENU_CONSOLE],
		     NULL, 9),
  /*
   * 09 
   SUBMENU_OPTION (&others_menu,
   */ SUBMENU_OPTION (NULL,
		      &msg[MSG_MAIN_MENU_STREAM],
		      NULL,
		      2),
};


void
main_menu_draw (PLAYER_CONTEXT * player_context)
{
  char line_buffer[512];
  MENU_OPTION_TYPE *menu_options = player_context->current_menu->options;

  if (player_context->redraw_menu == REDRAW_FULL)
    show_icon (down_screen_addr, &ICON_MAINBG, 0, 0);

  // Play
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_PLAY_SEL, 0, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_PLAY_NSEL, 0, 32);
    }

  menu_options++;

  // Pause
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_PAUSE_SEL, 34, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_PAUSE_NSEL, 34, 32);
    }

  menu_options++;

  // Stop
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_STOP_SEL, 68, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_STOP_NSEL, 68, 32);
    }

  menu_options++;

  // Start
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_START_SEL, 102, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_START_NSEL, 102, 32);
    }

  menu_options++;

  // RR
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_RR_SEL, 136, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_RR_NSEL, 136, 32);
    }

  menu_options++;

  // FF
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_FF_SEL, 170, 32);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_FF_NSEL, 170, 32);
    }

  menu_options++;

  // Info
  strcpy (line_buffer, *(menu_options->display_string));
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_SEL, 0, 160);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_NSEL, 0, 160);
    }
  draw_string_vcenter (down_screen_addr, 0, 176, 64, COLOR_BLACK,
		       line_buffer);

  menu_options++;

  // Load
  strcpy (line_buffer, *(menu_options->display_string));
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_SEL, 64, 160);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_NSEL, 64, 160);
    }
  draw_string_vcenter (down_screen_addr, 64, 176, 64, COLOR_BLACK,
		       line_buffer);

  menu_options++;

  // Console
  strcpy (line_buffer, *(menu_options->display_string));
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_SEL, 128, 160);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_NSEL, 128, 160);
    }
  draw_string_vcenter (down_screen_addr, 128, 176, 64, COLOR_BLACK,
		       line_buffer);

  menu_options++;

  // Streams
  strcpy (line_buffer, *(menu_options->display_string));
  if (menu_options == player_context->current_menu->option_to_focus)
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_SEL, 192, 160);
    }
  else if (player_context->redraw_menu == REDRAW_FULL
	   || (menu_options == player_context->current_menu->option_focused))
    {
      show_icon (down_screen_addr, &ICON_DOWN_BUTTON_NSEL, 192, 160);
    }
  draw_string_vcenter (down_screen_addr, 192, 176, 64, COLOR_BLACK,
		       line_buffer);

  player_context->current_menu->option_focused =
    player_context->current_menu->option_to_focus;
  player_context->redraw_menu = NO_REDRAW;
}

void
main_menu_key (gui_action_type gui_action, PLAYER_CONTEXT * player_context)
{
  int option_focused_num =
    OPTION_NUM (player_context->current_menu->option_focused,
		player_context->current_menu);


  switch (gui_action)
    {
    case CURSOR_DOWN:
      if (option_focused_num < 6)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options + option_focused_num / 2 + 6;
      break;

    case CURSOR_UP:
      if (option_focused_num >= 6 && option_focused_num < 9)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options + 2 * (option_focused_num -
						       6);
      break;

    case CURSOR_RIGHT:
      if (option_focused_num == 5)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options;
      else if (option_focused_num == 9)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options + 6;
      else
	player_context->current_menu->option_to_focus += 1;
      break;

    case CURSOR_LEFT:
      if (option_focused_num == 0)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options + 5;
      else if (option_focused_num == 6)
	player_context->current_menu->option_to_focus =
	  player_context->current_menu->options + 9;
      else
	player_context->current_menu->option_to_focus -= 1;
      break;

    case CURSOR_SELECT:
      player_context->action_function =
	player_context->current_menu->option_to_focus->action_function;
      player_context->new_menu =
	player_context->current_menu->option_to_focus->sub_menu;

    default:
      break;
    }				// end swith

  if (player_context->current_menu->option_to_focus !=
      player_context->current_menu->option_focused)
    player_context->redraw_menu = REDRAW_UPDATE;
}
