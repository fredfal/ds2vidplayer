/* gui.h
 *
 * Copyright (C) 2010 dking <dking024@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licens e as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef __GUI_H__
#define __GUI_H__

#include "ds2_types.h"
#include "fs_api.h"
#include "menu.h"

#define UP_SCREEN_UPDATE_METHOD   2
#define DOWN_SCREEN_UPDATE_METHOD 2

// For general option text
#define OPTION_TEXT_X             10
#define OPTION_TEXT_SX            236

// For option rows
#define GUI_ROW1_Y                36
#define GUI_ROW_SY                19
// The following offset is added to the row's Y coordinate to provide
// the Y coordinate for its text.
#define TEXT_OFFSET_Y             2
// The following offset is added to the row's Y coordinate to provide
// the Y coordinate for its ICON_SUBSELA (sub-screen selection type A).
#define SUBSELA_OFFSET_Y          -2
#define SUBSELA_X                 ((NDS_SCREEN_WIDTH - ICON_SUBSELA.x) / 2)

// For message boxes
#define MESSAGE_BOX_TEXT_X        ((NDS_SCREEN_WIDTH - ICON_MSG.x) / 2 + 3)
#define MESSAGE_BOX_TEXT_SX       (ICON_MSG.x - 6)
// Y is brought down by the "window title" that's part of ICON_MSG
#define MESSAGE_BOX_TEXT_Y        ((NDS_SCREEN_HEIGHT - ICON_MSG.y) / 2 + 24)

// For the file selector
#define FILE_SELECTOR_ICON_X      10
#define FILE_SELECTOR_ICON_Y      (TEXT_OFFSET_Y - 1)
#define FILE_SELECTOR_NAME_X      32
#define FILE_SELECTOR_NAME_SX     214

// Back button
#define BACK_BUTTON_X             229
#define BACK_BUTTON_Y             10
// Title icon
#define TITLE_ICON_X              12
#define TITLE_ICON_Y              9

#ifdef __cplusplus
extern "C"
{
#endif

  extern char main_path[MAX_PATH];

/******************************************************************************
 ******************************************************************************/
  extern char g_default_rom_dir[MAX_PATH];

/******************************************************************************
 ******************************************************************************/
  extern void gui_init (u32 lang_id);
  extern u32 menu ();

  extern void InitMessage (void);
  extern void FiniMessage (void);

  void menu_play (PLAYER_CONTEXT * player_context);
  void menu_exit (PLAYER_CONTEXT * player_context);
  void menu_loadfile (PLAYER_CONTEXT * player_context);
  void others_menu (PLAYER_CONTEXT * player_context);


#ifdef __cplusplus
}
#endif

#endif				//__GUI_H__
