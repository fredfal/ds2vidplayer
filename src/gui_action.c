#include <ds2io.h>
#include <ds2_types.h>
#include <ds2_timer.h>
#include "gui_action.h"

#define BUTTON_REPEAT_START (21428/2)
#define BUTTON_REPEAT_CONTINUE (21428/10)

u32 button_repeat_timestamp;

typedef enum
{
  BUTTON_NOT_HELD,
  BUTTON_HELD_INITIAL,
  BUTTON_HELD_REPEAT
} button_repeat_state_type;

button_repeat_state_type button_repeat_state = BUTTON_NOT_HELD;
unsigned int gui_button_repeat = 0;

GUI_ACTION_ENUM
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

void get_gui_input (GUI_ACTION_TYPE * gui_action)
{
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

                  gui_action->key_pressed = key_to_cursor (gui_keys[i]);
                  gui_action->x = inputdata.x;
                  gui_action->y = inputdata.y;

                  return;
                }
            }

          // No button pressed
          gui_action->key_pressed = CURSOR_NONE;
          return;
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
                  gui_action->key_pressed = CURSOR_NONE;
                  return;
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

                      gui_action->key_pressed = key_to_cursor (gui_keys[i]);
                      gui_action->x = inputdata.x;
                      gui_action->y = inputdata.y;
                      return;
                    }
                }
              // If it was time for the repeat but it
              // didn't occur, stop repeating.
              if (IsRepeatReady)
                button_repeat_state = BUTTON_NOT_HELD;
              gui_action->key_pressed = CURSOR_NONE;
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

