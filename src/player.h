#ifndef _PLAYER_H_
#define _PLAYER_H_

typedef enum
{
  STATUS_STOP,
  STATUS_PLAY,
} STATUS_TYPE_ENUM;

typedef enum
{
  REDRAW_UPDATE,
  NO_REDRAW,
  REDRAW_FULL,
} REDRAW_TYPE_ENUM;

struct _PLAYER_CONTEXT
{
  STATUS_TYPE_ENUM status;
  char filename[512];
  struct _MENU_TYPE *current_menu;
  struct _MENU_TYPE *new_menu;
  REDRAW_TYPE_ENUM redraw_menu;
  void (*action_function) (struct _PLAYER_CONTEXT * player_context);
};

typedef struct _PLAYER_CONTEXT PLAYER_CONTEXT;

int play_file (char *filename);

#endif
