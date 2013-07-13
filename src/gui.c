/*
 * gui.c This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version. This program is
 * distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details. You should have received a copy of the GNU General
 * Public License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA 
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "ds2_types.h"
#include "ds2_timer.h"
#include "ds2io.h"
#include "ds2_malloc.h"
#include "ds2_cpu.h"
#include "fs_api.h"
#include "key.h"
#include "gui.h"
#include "draw.h"
#include "message.h"
#include "bitmap.h"
#include "configfile.h"
#include "languagefile.h"
#include "main_menu.h"
#include "player.h"

char main_path[MAX_PATH];

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

/******************************************************************************
*	Macro definition
 ******************************************************************************/
#define DS2VIDPLAYER_VERSION "0.1a"

#define LANGUAGE_PACK   "SYSTEM/language.msg"
APPLICATION_CONFIG application_config;

/******************************************************************************
 ******************************************************************************/
char g_default_rom_dir[MAX_PATH];
/******************************************************************************
 ******************************************************************************/
static int sort_function (const void *dest_str_ptr, const void *src_str_ptr);
static void get_timestamp_string (char *buffer, u16 msg_id, u16 year,
				  u16 mon, u16 day, u16 wday, u16 hour,
				  u16 min, u16 sec, u32 msec);
static void get_time_string (char *buff, struct rtc *rtcp);
static void quit (void);

/*--------------------------------------------------------
	Get GUI input
--------------------------------------------------------*/
#define BUTTON_REPEAT_START (21428 / 2)
#define BUTTON_REPEAT_CONTINUE (21428 / 20)

u32 button_repeat_timestamp;

typedef enum
{
  BUTTON_NOT_HELD,
  BUTTON_HELD_INITIAL,
  BUTTON_HELD_REPEAT
} button_repeat_state_type;

button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;
unsigned int gui_button_repeat = 0;

gui_action_type
key_to_cursor (unsigned int key)
{
  switch (key)
    {
    case KEY_UP:
      return CURSOR_UP;
    case KEY_DOWN:
      return CURSOR_DOWN;
    case KEY_LEFT:
      return CURSOR_LEFT;
    case KEY_RIGHT:
      return CURSOR_RIGHT;
    case KEY_L:
      return CURSOR_LTRIGGER;
    case KEY_R:
      return CURSOR_RTRIGGER;
    case KEY_A:
      return CURSOR_SELECT;
    case KEY_B:
      return CURSOR_BACK;
    case KEY_X:
      return CURSOR_EXIT;
    case KEY_TOUCH:
      return CURSOR_TOUCH;
    default:
      return CURSOR_NONE;
    }
}

static unsigned int gui_keys[] = {
  KEY_A, KEY_B, KEY_X, KEY_L, KEY_R, KEY_TOUCH, KEY_UP, KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT
};

gui_action_type
get_gui_input (void)
{
  gui_action_type ret;

  struct key_buf inputdata;
  ds2_getrawInput (&inputdata);

  if (inputdata.key & KEY_LID)
    {
      ds2_setSupend ();
      do
	{
	  ds2_getrawInput (&inputdata);
	  mdelay (1);
	}
      while (inputdata.key & KEY_LID);
      ds2_wakeup ();
    }

  unsigned int i;
  while (1)
    {
      switch (button_repeat_state)
	{
	case BUTTON_NOT_HELD:
	  // Pick the first pressed button out of the gui_keys array.
	  for (i = 0; i < sizeof (gui_keys) / sizeof (gui_keys[0]); i++)
	    {
	      if (inputdata.key & gui_keys[i])
		{
		  button_repeat_state = BUTTON_HELD_INITIAL;
		  button_repeat_timestamp = getSysTime ();
		  gui_button_repeat = gui_keys[i];
		  return key_to_cursor (gui_keys[i]);
		}
	    }
	  return CURSOR_NONE;
	case BUTTON_HELD_INITIAL:
	case BUTTON_HELD_REPEAT:
	  // If the key that was being held isn't anymore...
	  if (!(inputdata.key & gui_button_repeat))
	    {
	      button_repeat_state = BUTTON_NOT_HELD;
	      // Go see if another key is held (try #2)
	      break;
	    }
	  else
	    {
	      unsigned int IsRepeatReady =
		getSysTime () - button_repeat_timestamp >=
		(button_repeat_state ==
		 BUTTON_HELD_INITIAL ? BUTTON_REPEAT_START :
		 BUTTON_REPEAT_CONTINUE);
	      if (!IsRepeatReady)
		{
		  // Temporarily turn off the key.
		  // It's not its turn to be repeated.
		  inputdata.key &= ~gui_button_repeat;
		}
	      // Pick the first pressed button out of the gui_keys
	      // array.
	      for (i = 0; i < sizeof (gui_keys) / sizeof (gui_keys[0]); i++)
		{
		  if (inputdata.key & gui_keys[i])
		    {
		      // If it's the held key,
		      // it's now repeating quickly.
		      button_repeat_state =
			gui_keys[i] ==
			gui_button_repeat ?
			BUTTON_HELD_REPEAT : BUTTON_HELD_INITIAL;
		      button_repeat_timestamp = getSysTime ();
		      gui_button_repeat = gui_keys[i];
		      return key_to_cursor (gui_keys[i]);
		    }
		}
	      // If it was time for the repeat but it
	      // didn't occur, stop repeating.
	      if (IsRepeatReady)
		button_repeat_state = BUTTON_NOT_HELD;
	      return CURSOR_NONE;
	    }
	}
    }
}

/*--------------------------------------------------------
	Wait any key in [key_list] pressed
	if key_list == NULL, return at any key pressed
--------------------------------------------------------*/
unsigned int
wait_Anykey_press (unsigned int key_list)
{
  unsigned int key;

  while (1)
    {
      key = getKey ();
      if (key)
	{
	  if (0 == key_list)
	    break;
	  else if (key & key_list)
	    break;
	}
    }

  return key;
}

/*--------------------------------------------------------
	Wait all key in [key_list] released
	if key_list == NULL, return at all key released
--------------------------------------------------------*/
void
wait_Allkey_release (unsigned int key_list)
{
  unsigned int key;
  struct key_buf inputdata;

  while (1)
    {
      ds2_getrawInput (&inputdata);
      key = inputdata.key;

      if (0 == key)
	break;
      else if (!key_list)
	continue;
      else if (0 == (key_list & key))
	break;
    }
}

void
InitMessage (void)
{
  ds2_setCPUclocklevel (0);
  draw_message (down_screen_addr, NULL, 28, 31, 227, 165, COLOR_BG);
}

void
FiniMessage (void)
{
  wait_Allkey_release (0);
  ds2_setCPUclocklevel (13);
}

unsigned int
ReadInputDuringCompression ()
{
  struct key_buf inputdata;

  ds2_getrawInput (&inputdata);

  return inputdata.key & ~(KEY_LID);
}

void
change_ext (char *src, char *buffer, char *extension)
{
  char *dot_position;

  strcpy (buffer, src);
  dot_position = strrchr (buffer, '.');

  if (dot_position)
    strcpy (dot_position, extension);
}

void
menu_play (PLAYER_CONTEXT * player_context)
{
  char line_buffer[512];

  if (player_context->filename != NULL)
    {
      strcpy (line_buffer, g_default_rom_dir);
      strcat (line_buffer, "/");
      strcat (line_buffer, player_context->filename);

      player_context->status = 1;
      init_file (line_buffer);
    }
}

void
choose_menu (MENU_TYPE * new_menu, PLAYER_CONTEXT * player_context)
{
  if (NULL != player_context->current_menu)
    {
      if (player_context->current_menu->end_function)
	player_context->current_menu->end_function (player_context);
    }

  player_context->current_menu = new_menu;
  player_context->redraw_menu = REDRAW_FULL;
}

void
menu_loadfile (PLAYER_CONTEXT * player_context)
{
  char *file_ext[] = { ".avi" };
  int return_value;

  return_value = load_file (file_ext, player_context->filename, g_default_rom_dir);

  printf("%d\n", return_value);
  choose_menu (player_context->current_menu, player_context);
}

void
menu_exit (PLAYER_CONTEXT * player_context)
{
  ds2_setCPUclocklevel (13);	// Crank it up, leave quickly
  quit ();
}

/*--------------------------------------------------------
	Main Menu
--------------------------------------------------------*/
u32
menu ()
{
  gui_action_type gui_action;
  u32 i;
  u32 repeat;
  u32 return_value = 0;
  char tmp_filename[MAX_FILE];
  char line_buffer[512];
  u16 *bg_screenp;
  u32 bg_screenp_color;

  PLAYER_CONTEXT player_context = { STATUS_STOP, NULL, NULL, NO_REDRAW };

  auto void others_menu_end ();
  auto void check_application_version ();
  auto void language_set ();

  // Local function definition


  void load_default_setting ()
  {
    if (bg_screenp != NULL)
      {
	bg_screenp_color = COLOR16 (43, 11, 11);
	memcpy (bg_screenp, down_screen_addr, 256 * 192 * 2);
      }
    else
      bg_screenp_color = COLOR_BG;

    draw_message (down_screen_addr, bg_screenp, 28, 31, 227, 165,
		  bg_screenp_color);
    draw_string_vcenter (down_screen_addr, MESSAGE_BOX_TEXT_X,
			 MESSAGE_BOX_TEXT_Y, MESSAGE_BOX_TEXT_SX,
			 COLOR_MSSG, msg[MSG_DIALOG_RESET]);

    if (draw_yesno_dialog
	(DOWN_SCREEN, 115, msg[MSG_GENERAL_CONFIRM_WITH_A],
	 msg[MSG_GENERAL_CANCEL_WITH_B]))
      {
	wait_Allkey_release (0);
	draw_message (down_screen_addr, bg_screenp, 28, 31, 227,
		      165, bg_screenp_color);
	draw_string_vcenter (down_screen_addr,
			     MESSAGE_BOX_TEXT_X,
			     MESSAGE_BOX_TEXT_Y,
			     MESSAGE_BOX_TEXT_SX, COLOR_MSSG,
			     msg[MSG_PROGRESS_RESETTING]);
	ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

	sprintf (line_buffer, "%s/%s", main_path,
		 APPLICATION_CONFIG_FILENAME);
	remove (line_buffer);

	init_application_config (&application_config);

	ds2_clearScreen (UP_SCREEN, 0);
	ds2_flipScreen (UP_SCREEN, 1);
      }
  }

  void check_application_version ()
  {
    if (bg_screenp != NULL)
      {
	bg_screenp_color = COLOR16 (43, 11, 11);
	memcpy (bg_screenp, down_screen_addr, 256 * 192 * 2);
      }
    else
      bg_screenp_color = COLOR_BG;

    draw_message (down_screen_addr, bg_screenp, 28, 31, 227, 165,
		  bg_screenp_color);
    sprintf (line_buffer, "%s\n%s %s", msg[MSG_APPLICATION_NAME],
	     msg[MSG_WORD_APPLICATION_VERSION], DS2VIDPLAYER_VERSION);
    draw_string_vcenter (down_screen_addr, MESSAGE_BOX_TEXT_X,
			 MESSAGE_BOX_TEXT_Y, MESSAGE_BOX_TEXT_SX,
			 COLOR_MSSG, line_buffer);
    ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

    wait_Allkey_release (0);	// invoked from the menu
    wait_Anykey_press (0);	// wait until the user presses something
    wait_Allkey_release (0);	// don't give that button to the menu
  }

  void language_set ()
  {
    if (gui_action == CURSOR_LEFT || gui_action == CURSOR_RIGHT)
      {
	ds2_setCPUclocklevel (13);	// crank it up
	if (bg_screenp != NULL)
	  {
	    bg_screenp_color = COLOR16 (43, 11, 11);
	    memcpy (bg_screenp, down_screen_addr, 256 * 192 * 2);
	  }
	else
	  bg_screenp_color = COLOR_BG;

	load_language_msg (main_path, LANGUAGE_PACK,
			   application_config.language);

	save_application_config_file (&application_config, main_path);
	ds2_setCPUclocklevel (0);	// and back down
      }
  }

  char *on_off_options[] =
    { (char *) &msg[MSG_GENERAL_OFF], (char *) &msg[MSG_GENERAL_ON] };

  /*MAKE_MENU (others, NULL, option_menu_draw, option_menu_key, others_menu_end, 1, 1); */
  //MENU_TYPE main_menu = MAKE_MENU (main_menu_draw, main_menu_key, NULL, main_menu_options, main_menu_options, main_menu_options);
  MENU_TYPE main_menu = {
    main_menu_draw,
    main_menu_key,
    NULL,
    main_menu_options,
    sizeof (main_menu_options) / sizeof (MENU_OPTION_TYPE),
    main_menu_options, main_menu_options
  };

  void tools_menu_init ()
  {
  }

  void others_menu_end ()
  {
    save_application_config_file (&application_config, main_path);
  }

  // ----------------------------------------------------------------------------//
  // Menu Start
  ds2_setCPUclocklevel (0);

  wait_Allkey_release (~KEY_LID);	// Allow the lid closing to go
  // through
  // so the user can close the lid and make it sleep after compressing
  bg_screenp = (u16 *) malloc (256 * 192 * 2);

  repeat = 1;

  ds2_clearScreen (UP_SCREEN, RGB15 (0, 0, 0));
  ds2_flipScreen (UP_SCREEN, UP_SCREEN_UPDATE_METHOD);

  choose_menu (&main_menu, &player_context);

  char message[512];
  // Menu loop
  err_msg (DOWN_SCREEN, "TEST");

  while (repeat)
    {
      struct key_buf inputdata;

      gui_action = get_gui_input ();

      if (player_context.status == STATUS_PLAY)
	{
	  ds2_setCPUclocklevel (13);
	  play_file (player_context.filename);
	}
      else
	{
	  ds2_setCPUclocklevel (0);
	  mdelay (20);		// to prevent the DSTwo-DS link from being 
	  // too clogged
	  // to return button statuses
	}

      switch (gui_action)
	{
	  //case CURSOR_TOUCH:
	  // ds2_getrawInput (&inputdata);
	  /*
	   * Back button at the top of every menu but the main one 
	   */
	  /*if (current_menu != &main_menu
	     && inputdata.x >= BACK_BUTTON_X
	     && inputdata.y < BACK_BUTTON_Y + ICON_BACK.y)
	     {
	     choose_menu (current_menu->options->sub_menu);
	     break;
	     } */
	  /*
	   * Main menu 
	   */
	  /*if (current_menu == &main_menu)
	     {
	     // 0 128 256
	     // _____ _____ 0
	     // |0CMP_|1DEC_| 96
	     // |2OPT_|3EXI_| 192

	     current_option_num =
	     (inputdata.y / 96) * 2 + (inputdata.x / 128);
	     current_option = current_menu->options + current_option_num;

	     if (current_option->option_type & HIDEN_TYPE)
	     break;
	     else if (current_option->option_type & ACTION_TYPE)
	     current_option->action_function ();
	     else if (current_option->option_type & SUBMENU_TYPE)
	     choose_menu (current_option->sub_menu);
	     } */
	  /*
	   * This is the majority case, covering all menus except file
	   * loading 
	   */
	  /*else if (current_menu !=
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
	     u32 next_option_num =
	     (inputdata.y - GUI_ROW1_Y) / GUI_ROW_SY + 1;
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

	     if (current_menu->gui_action_function)
	     {
	     gui_action = CURSOR_RIGHT;
	     current_menu->gui_action_function();
	     }
	     else if (current_option->option_type &
	     (NUMBER_SELECTION_TYPE | STRING_SELECTION_TYPE))
	     {
	     menu_default_keys ();
	     }
	     else if (current_option->option_type & ACTION_TYPE)
	     current_option->action_function ();
	     else if (current_option->option_type & SUBMENU_TYPE)
	     choose_menu (current_option->sub_menu);
	     }
	     break; */

	case CURSOR_DOWN:
	case CURSOR_UP:
	case CURSOR_RIGHT:
	case CURSOR_LEFT:
	case CURSOR_SELECT:
	  player_context.current_menu->gui_action_function (gui_action,
							    &player_context);
	  break;

	case CURSOR_EXIT:
	  break;

	case CURSOR_BACK:
	  if (player_context.current_menu != &main_menu)
	    choose_menu (player_context.current_menu->options->sub_menu,
			 &player_context);
	  break;

	default:
	  break;
	}			// end swith

      // Redrawing menu screen if needed
      if (player_context.redraw_menu == REDRAW_UPDATE
	  || player_context.redraw_menu == REDRAW_FULL)
	{
	  player_context.current_menu->draw_function (&player_context);

	  if (player_context.status == STATUS_PLAY)
	    ds2_flipScreen (DOWN_SCREEN, 0);
	  else
	    ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

	  player_context.redraw_menu = NO_REDRAW;
	}

      // Executing action if needed
      if (player_context.action_function != NULL)
	{
	  player_context.action_function (&player_context);
	  player_context.action_function = NULL;
	}
    }				// end while

  //if (current_menu && current_menu->end_function)
  //  current_menu->end_function ();

  if (bg_screenp != NULL)
    free ((void *) bg_screenp);

  ds2_clearScreen (DOWN_SCREEN, 0);
  ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
  wait_Allkey_release (0);

  ds2_setCPUclocklevel (13);

  return return_value;
}

/*--------------------------------------------------------
	Load font library
--------------------------------------------------------*/
u32
load_font ()
{
  return (u32) BDF_font_init ();
}

void
quit (void)
{
  /*
   * u32 reg_ra;
   * 
   * __asm__ __volatile__("or %0, $0, $ra" : "=r" (reg_ra) :);
   * 
   * dbg_printf("return address= %08x\n", reg_ra); 
   */

#ifdef USE_DEBUG
  fclose (g_dbg_file);
#endif

  ds2_plug_exit ();
  while (1);
}

/*
 *      Function: search directory on directory_path
 *      directory: directory name will be searched
 *      directory_path: path, note that the buffer which hold directory_path should
 *              be large enough for nested
 *      return: 0= found, directory lay on directory_path
 */
int
search_dir (char *directory, char *directory_path)
{
  DIR *current_dir;
  dirent *current_file;
  struct stat st;
  int directory_path_len;

  current_dir = opendir (directory_path);
  if (current_dir == NULL)
    return -1;

  directory_path_len = strlen (directory_path);

  // while((current_file = readdir(current_dir)) != NULL)
  while ((current_file = readdir_ex (current_dir, &st)) != NULL)
    {
      // Is directory 
      if (S_ISDIR (st.st_mode))
	{
	  if (strcmp (".", current_file->d_name)
	      || strcmp ("..", current_file->d_name))
	    continue;

	  strcpy (directory_path + directory_path_len, current_file->d_name);

	  if (!strcasecmp (current_file->d_name, directory))
	    {			// dirctory 
	      // find
	      closedir (current_dir);
	      return 0;
	    }

	  if (search_dir (directory, directory_path) == 0)
	    {			// dirctory 
	      // find
	      closedir (current_dir);
	      return 0;
	    }

	  directory_path[directory_path_len] = '\0';
	}
    }

  closedir (current_dir);
  return -1;
}


u32
file_length (FILE * file)
{
  u32 pos, size;
  pos = ftell (file);
  fseek (file, 0, SEEK_END);
  size = ftell (file);
  fseek (file, pos, SEEK_SET);

  return size;
}

/*
 *      GUI Initialize
 */
void
gui_init (u32 lang_id)
{
  int flag;

  ds2_setCPUclocklevel (13);	// Crank it up. When the menu starts, ->
  // 0.

  // Find the "DS2VIDPLAYER" system directory
  DIR *current_dir;

  strcpy (main_path, "fat:/DS2VIDPLAYER");
  current_dir = opendir (main_path);
  if (current_dir)
    closedir (current_dir);
  else
    {
      strcpy (main_path, "fat:/_SYSTEM/PLUGINS/DS2VIDPLAYER");
      current_dir = opendir (main_path);
      if (current_dir)
	closedir (current_dir);
      else
	{
	  strcpy (main_path, "fat:");
	  if (search_dir ("DS2VIDPLAYER", main_path) == 0)
	    {
	      printf
		("Found DS2VIDPLAYER directory\nDossier DS2VIDPLAYER trouve\n\n%s\n",
		 main_path);
	    }
	  else
	    {
	      err_msg (DOWN_SCREEN,
		       "/DS2VIDPLAYER: Directory missing\nPress any key to return to\nthe menu\n\n/DS2VIDPLAYER: Dossier manquant\nAppuyer sur une touche pour\nretourner au menu");
	      goto gui_init_err;
	    }
	}
    }

  show_log (down_screen_addr);
  ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

  flag = icon_init (lang_id);
  if (0 != flag)
    {
      err_msg (DOWN_SCREEN,
	       "Some icons are missing\nLoad them onto your card\nPress any key to return to\nthe menu\n\nDes icones sont manquantes\nChargez-les sur votre carte\nAppuyer sur une touche pour\nretourner au menu");
      goto gui_init_err;
    }

  flag = load_font ();
  if (0 != flag)
    {
      char message[512];
      sprintf (message,
	       "Font library initialisation\nerror (%d)\nPress any key to return to\nthe menu\n\nErreur d'initalisation de la\npolice de caracteres (%d)\nAppuyer sur une touche pour\nretourner au menu",
	       flag, flag);
      err_msg (DOWN_SCREEN, message);
      goto gui_init_err;
    }

  load_application_config_file (&application_config, main_path);
  lang_id = application_config.language;

  flag = load_language_msg (main_path, LANGUAGE_PACK, lang_id);
  if (0 != flag)
    {
      char message[512];
      sprintf (message,
	       "Language pack initialisation\nerror (%d)\nPress any key to return to\nthe menu\n\nErreur d'initalisation du\npack de langue (%d)\nAppuyer sur une touche pour\nretourner au menu",
	       flag, flag);
      err_msg (DOWN_SCREEN, message);
      goto gui_init_err;
    }

  strcpy (g_default_rom_dir, "fat:");

  return;

gui_init_err:
  ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
  wait_Anykey_press (0);
  quit ();
}
