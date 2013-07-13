#include "message.h"

// If adding a language, make sure you update the size of the array in
// message.h too.
char *lang[4] = {
  "English",			// 0
  "Français",			// 1
  "Español",			// 2
  "Deutsch",			// 3
};

char *language_options[] =
  { (char *) &lang[0], (char *) &lang[1], (char *) &lang[2],
  (char *) &lang[3]
};

MENU_OPTION_TYPE others_options[] = {
  /*
   * 00 
   */ SUBMENU_OPTION (NULL, &msg[MSG_MAIN_MENU_OPTIONS], NULL,
		      0),
  /*
   * 01 
   */ NUMERIC_SELECTION_OPTION (NULL,
				&msg[FMT_OPTIONS_COMPRESSION_LEVEL],
				&application_config.CompressionLevel,
				10, NULL, 1),
  /*
   * 02 
   */ STRING_SELECTION_OPTION (language_set, NULL,
			       &msg[FMT_OPTIONS_LANGUAGE],
			       language_values,
			       &application_config.language,
			       sizeof (language_options) /
			       sizeof (language_options[0])
			       /*
			        * number of possible languages 
			        */
			       ,
			       NULL, ACTION_TYPE, 2),
  /*
   * 03 
   */ STRING_SELECTION_OPTION (NULL, show_card_space,
			       &msg[MSG_OPTIONS_CARD_CAPACITY],
			       NULL,
			       &desert, 2, NULL,
			       PASSIVE_TYPE | HIDEN_TYPE, 3),
  /*
   * 04 
   */ ACTION_OPTION (load_default_setting, NULL,
		     &msg[MSG_OPTIONS_RESET], NULL, 4),
  /*
   * 05 
   */ ACTION_OPTION (check_application_version, NULL,
		     &msg[MSG_OPTIONS_VERSION], NULL, 5),
};


void
option_menu_draw (PLAYER_CONTEXT * player_context, int full_draw)
{
  u32 line_num, screen_focus, focus_option;

  if (full_draw)
    {
      // draw background
      show_icon (down_screen_addr, &ICON_SUBBG, 0, 0);
      show_icon (down_screen_addr, &ICON_TITLE, 0, 0);
      show_icon (down_screen_addr, &ICON_TITLEICON, TITLE_ICON_X,
		 TITLE_ICON_Y);

      strcpy (line_buffer, *(display_option->display_string));
      draw_string_vcenter (down_screen_addr, 0, 9, 256,
			   COLOR_ACTIVE_ITEM, line_buffer);
    }

  line_num = current_option_num;
  screen_focus = player_context->current_menu->screen_focus;
  focus_option = player_context->current_menu->focus_option;

  if (focus_option < line_num)	// focus next option
    {
      focus_option = line_num - focus_option;
      screen_focus += focus_option;
      if (screen_focus > SUBMENU_ROW_NUM)	// Reach max row numbers
	// can display
	screen_focus = SUBMENU_ROW_NUM;

      current_menu->screen_focus = screen_focus;
      focus_option = line_num;
    }
  else if (focus_option > line_num)	// focus last option
    {
      focus_option = focus_option - line_num;
      if (screen_focus > focus_option)
	screen_focus -= focus_option;
      else
	screen_focus = 0;

      if (screen_focus == 0 && line_num > 0)
	screen_focus = 1;

      current_menu->screen_focus = screen_focus;
      focus_option = line_num;
    }
  current_menu->focus_option = focus_option;

  i = focus_option - screen_focus;
  display_option += i + 1;

  line_num = current_menu->num_options - 1;
  if (line_num > SUBMENU_ROW_NUM)
    line_num = SUBMENU_ROW_NUM;

  if (focus_option == 0)
    show_icon (down_screen_addr, &ICON_BACK, BACK_BUTTON_X, BACK_BUTTON_Y);
  else
    show_icon (down_screen_addr, &ICON_NBACK, BACK_BUTTON_X, BACK_BUTTON_Y);

  for (i = 0; i < line_num; i++, display_option++)
    {
      unsigned short color;

      if (display_option == current_option)
	show_icon (down_screen_addr, &ICON_SUBSELA,
		   SUBSELA_X, GUI_ROW1_Y + i * GUI_ROW_SY + SUBSELA_OFFSET_Y);

      if (display_option->option_type & NUMBER_SELECTION_TYPE)
	{
	  sprintf (line_buffer,
		   *(display_option->display_string),
		   *(display_option->current_option));
	}
      else if (display_option->option_type & STRING_SELECTION_TYPE)
	{
	  sprintf (line_buffer,
		   *(display_option->display_string),
		   *((u32 *) (((u32 *)
			       display_option->options)[*
							(display_option->current_option)])));
	}
      else
	{
	  strcpy (line_buffer, *(display_option->display_string));
	}

      if (display_option->passive_function == NULL)
	{
	  if (display_option == current_option)
	    color = COLOR_ACTIVE_ITEM;
	  else
	    color = COLOR_INACTIVE_ITEM;

	  PRINT_STRING_BG (down_screen_addr, line_buffer,
			   color, COLOR_TRANS,
			   OPTION_TEXT_X,
			   GUI_ROW1_Y + i * GUI_ROW_SY + TEXT_OFFSET_Y);
	}
    }
}

MENU_UPDATE
option_menu_keys (PLAYER_CONTEXT * player_context, GUI_ACTION gui_action)
{
  struct key_buf inputdata;

  switch (gui_action)
    {
    case CURSOR_TOUCH:
      ds2_getrawInput (&inputdata);
      /*
       * Back button at the top of every menu but the main one 
       */
      if (inputdata.x >= BACK_BUTTON_X
	  && inputdata.y < BACK_BUTTON_Y + ICON_BACK.y)
	{
	  choose_menu (current_menu->options->sub_menu);
	  break;
	}

      /*
       * This is the majority case, covering all menus except file
       * loading 
       */
      if (current_menu !=
	  (main_menu.options + 0)->sub_menu
	  && current_menu != (main_menu.options + 1)->sub_menu)
	{
	  if (inputdata.y <= GUI_ROW1_Y
	      || inputdata.y > GUI_ROW1_Y + GUI_ROW_SY * SUBMENU_ROW_NUM)
	    break;
	  // ___ 33 This screen has 6 possible rows. Touches
	  // ___ 60 above or below these are ignored.
	  // . . . (+27) The row between 33 and 60 is [1], though!
	  // ___ 192
	  u32 next_option_num = (inputdata.y - GUI_ROW1_Y) / GUI_ROW_SY + 1;
	  struct _MENU_OPTION_TYPE *next_option =
	    current_menu->options + next_option_num;

	  if (next_option_num >= current_menu->num_options)
	    break;

	  if (!next_option)
	    break;

	  if (next_option->option_type & HIDEN_TYPE)
	    break;

	  current_option_num = next_option_num;
	  current_option = current_menu->options + current_option_num;

	  if (current_menu->key_function)
	    {
	      gui_action = CURSOR_RIGHT;
	      current_menu->key_function ();
	    }
	  else if (current_option->option_type &
		   (NUMBER_SELECTION_TYPE | STRING_SELECTION_TYPE))
	    {
	      gui_action = CURSOR_RIGHT;
	      u32 current_option_val = *(current_option->current_option);

	      if (current_option_val < current_option->num_options - 1)
		current_option_val++;
	      else
		current_option_val = 0;
	      *(current_option->current_option) = current_option_val;

	      if (current_option->action_function)
		current_option->action_function ();
	    }
	  else if (current_option->option_type & ACTION_TYPE)
	    current_option->action_function ();
	  else if (current_option->option_type & SUBMENU_TYPE)
	    choose_menu (current_option->sub_menu);
	}
      break;
    case CURSOR_DOWN:
      current_option_num =
	(current_option_num + 1) % current_menu->num_options;
      current_option = current_menu->options + current_option_num;

      while (current_option->option_type & HIDEN_TYPE)
	{
	  current_option_num =
	    (current_option_num + 1) % current_menu->num_options;
	  current_option = current_menu->options + current_option_num;
	}
      break;
    case CURSOR_UP:
      if (current_menu->key_function)
	current_menu->key_function ();
      else
	{
	  if (current_option_num)
	    current_option_num--;
	  else
	    current_option_num = current_menu->num_options - 1;
	  current_option = current_menu->options + current_option_num;

	  while (current_option->option_type & HIDEN_TYPE)
	    {
	      if (current_option_num)
		current_option_num--;
	      else
		current_option_num = current_menu->num_options - 1;
	      current_option = current_menu->options + current_option_num;
	    }
	}
      break;

    case CURSOR_RIGHT:
      if (current_option->option_type & (NUMBER_SELECTION_TYPE |
					 STRING_SELECTION_TYPE))
	{
	  u32 current_option_val = *(current_option->current_option);

	  if (current_option_val < current_option->num_options - 1)
	    current_option_val++;
	  else
	    current_option_val = 0;
	  *(current_option->current_option) = current_option_val;

	  if (current_option->action_function)
	    current_option->action_function ();
	}
      break;

    case CURSOR_LEFT:
      if (current_option->option_type & (NUMBER_SELECTION_TYPE |
					 STRING_SELECTION_TYPE))
	{
	  u32 current_option_val = *(current_option->current_option);

	  if (current_option_val)
	    current_option_val--;
	  else
	    current_option_val = current_option->num_options - 1;
	  *(current_option->current_option) = current_option_val;

	  if (current_option->action_function)
	    current_option->action_function ();
	}
      break;

    case CURSOR_BACK:
      choose_menu (current_menu->options->sub_menu);
      break;

    default:
      break;
    }
}
