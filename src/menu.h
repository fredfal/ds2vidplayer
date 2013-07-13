#ifndef __MENU_H__
#define __MENU_H__

#include "player.h"
#include "gui_action.h"

typedef enum
{
  NUMBER_SELECTION_TYPE = 0x01,
  STRING_SELECTION_TYPE = 0x02,
  SUBMENU_TYPE = 0x04,
  ACTION_TYPE = 0x08,
  HIDEN_TYPE = 0x10,
  PASSIVE_TYPE = 0x00,
} MENU_OPTION_TYPE_ENUM;

struct _MENU_OPTION_TYPE
{
  void (*action_function) ();	// Active option to process input
  struct _MENU_TYPE *sub_menu;	// Sub-menu of this option
  char **display_string;	// Name and other things of this
  // option
  void *values;			// output value of this option
  u32 *current_value;		// output values
  u32 num_values;		// Total output values
  char **help_string;		// Help string
  u32 line_number;		// Order id of this option in it
  // menu
  MENU_OPTION_TYPE_ENUM option_type;	// Option types
};

typedef struct _MENU_OPTION_TYPE MENU_OPTION_TYPE;

struct _MENU_TYPE
{
  void (*draw_function) (PLAYER_CONTEXT * player_context);	// Function to update this menu
  void (*gui_action_function) (gui_action_type gui_action, PLAYER_CONTEXT * player_context);	// Function to process input
  void (*end_function) (PLAYER_CONTEXT * player_context);	// End process of this menu
  MENU_OPTION_TYPE *options;	// Options array
  u32 num_options;		// Total options of this menu
  MENU_OPTION_TYPE *option_to_focus;	// Option which obtained focus
  MENU_OPTION_TYPE *option_focused;	// screen positon of the focus
  // option
};

typedef struct _MENU_TYPE MENU_TYPE;

#define SUBMENU_ROW_NUM 8
#define FILE_LIST_ROWS 8

#define OPTION_NUM(option, menu) \
  (option - menu->options)

#define MAKE_MENU(draw_function, gui_action_function, end_function, options, option_to_focus, option_focused) \
  {                                                                           \
    draw_function,                                                            \
    gui_action_function,                                                             \
    end_function,                                                             \
    options,                                                           \
    sizeof(options) / sizeof(MENU_OPTION_TYPE),                        \
    option_to_focus,                                                             \
    option_focused                                                              \
  }                                                                           \

#define ACTION_OPTION(action_function, display_string,      \
 help_string, line_number)                                                    \
{                                                                             \
  action_function,                                                            \
  NULL,                                                                       \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  0,                                                                          \
  help_string,                                                                \
  line_number,                                                                \
  ACTION_TYPE                                                                 \
}                                                                             \

#define SUBMENU_OPTION(sub_menu, display_string, help_string, line_number)    \
{                                                                             \
  NULL,                                                                       \
  sub_menu,                                                                   \
  display_string,                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  sizeof(sub_menu) / sizeof(MENU_OPTION_TYPE),                                \
  help_string,                                                                \
  line_number,                                                                \
  SUBMENU_TYPE                                                                \
}                                                                             \

#define SELECTION_OPTION(display_string, options,           \
 option_ptr, num_options, help_string, line_number, type)                     \
{                                                                             \
  NULL,                                                                       \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type                                                                        \
}                                                                             \

#define ACTION_SELECTION_OPTION(action_function,            \
 display_string, options, option_ptr, num_options, help_string, line_number,  \
 type)                                                                        \
{                                                                             \
  action_function,                                                            \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  type | ACTION_TYPE                                                          \
}                                                                             \

#define STRING_SELECTION_OPTION(action_function,            \
    display_string, options, option_ptr, num_options, help_string, action, line_number)\
{                                                                             \
  action_function,                                                            \
  NULL,                                                                       \
  display_string,                                                             \
  options,                                                                    \
  option_ptr,                                                                 \
  num_options,                                                                \
  help_string,                                                                \
  line_number,                                                                \
  STRING_SELECTION_TYPE | action                                              \
}

#define NUMERIC_SELECTION_OPTION(display_string,            \
 option_ptr, num_options, help_string, line_number)                           \
  SELECTION_OPTION(display_string, NULL, option_ptr,        \
   num_options, help_string, line_number, NUMBER_SELECTION_TYPE)              \

#define STRING_SELECTION_HIDEN_OPTION(action_function,       \
 display_string, options, option_ptr, num_options, help_string, line_number)  \
  ACTION_SELECTION_OPTION(action_function,                  \
   display_string,  options, option_ptr, num_options, help_string,            \
   line_number, (STRING_SELECTION_TYPE | HIDEN_TYPE))                         \

#define NUMERIC_SELECTION_ACTION_OPTION(action_function,    \
 display_string, option_ptr, num_options, help_string, line_number)           \
  ACTION_SELECTION_OPTION(action_function,                  \
   display_string,  NULL, option_ptr, num_options, help_string,               \
   line_number, NUMBER_SELECTION_TYPE)                                        \

#define NUMERIC_SELECTION_HIDE_OPTION(action_function,      \
    display_string, option_ptr, num_options, help_string, line_number)        \
  ACTION_SELECTION_OPTION(action_function,                   \
   display_string, NULL, option_ptr, num_options, help_string,                \
   line_number, NUMBER_SELECTION_TYPE)                                        \

#endif
