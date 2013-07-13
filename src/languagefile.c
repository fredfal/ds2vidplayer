#include "ds2_types.h"
#include "ds2_timer.h"
#include "ds2io.h"
#include "ds2_malloc.h"
#include "ds2_cpu.h"
#include "fs_api.h"
#include "key.h"
#include "message.h"

// These are U+05C8 and subsequent codepoints encoded in UTF-8.
const char HOTKEY_A_DISPLAY[] = { 0xD7, 0x88, 0x00 };
const char HOTKEY_B_DISPLAY[] = { 0xD7, 0x89, 0x00 };
const char HOTKEY_X_DISPLAY[] = { 0xD7, 0x8A, 0x00 };
const char HOTKEY_Y_DISPLAY[] = { 0xD7, 0x8B, 0x00 };
const char HOTKEY_L_DISPLAY[] = { 0xD7, 0x8C, 0x00 };
const char HOTKEY_R_DISPLAY[] = { 0xD7, 0x8D, 0x00 };
const char HOTKEY_START_DISPLAY[] = { 0xD7, 0x8E, 0x00 };
const char HOTKEY_SELECT_DISPLAY[] = { 0xD7, 0x8F, 0x00 };

// These are U+2190 and subsequent codepoints encoded in UTF-8.
const char HOTKEY_LEFT_DISPLAY[] = { 0xE2, 0x86, 0x90, 0x00 };
const char HOTKEY_UP_DISPLAY[] = { 0xE2, 0x86, 0x91, 0x00 };
const char HOTKEY_RIGHT_DISPLAY[] = { 0xE2, 0x86, 0x92, 0x00 };
const char HOTKEY_DOWN_DISPLAY[] = { 0xE2, 0x86, 0x93, 0x00 };

/*--------------------------------------------------------
        Load language message
--------------------------------------------------------*/
int
load_language_msg (char *path, char *filename, u32 language)
{
  FILE *fp;
  char msg_path[MAX_PATH];
  char string[256];
  char start[32];
  char end[32];
  char *pt, *dst;
  u32 loop, offset, len;
  int ret;

  sprintf (msg_path, "%s/%s", path, filename);
  fp = fopen (msg_path, "rb");
  if (fp == NULL)
    return -1;

  switch (language)
    {
    case ENGLISH:
    default:
      strcpy (start, "STARTENGLISH");
      strcpy (end, "ENDENGLISH");
      break;
    case FRENCH:
      strcpy (start, "STARTFRENCH");
      strcpy (end, "ENDFRENCH");
      break;
    case SPANISH:
      strcpy (start, "STARTSPANISH");
      strcpy (end, "ENDSPANISH");
      break;
    case GERMAN:
      strcpy (start, "STARTGERMAN");
      strcpy (end, "ENDGERMAN");
      break;
    }
  u32 cmplen = strlen (start);

  //find the start flag
  ret = 0;
  while (1)
    {
      pt = fgets (string, 256, fp);
      if (pt == NULL)
	{
	  ret = -2;
	  goto load_language_msg_error;
	}

      if (!strncmp (pt, start, cmplen))
	break;
    }

  loop = 0;
  offset = 0;
  dst = msg_data;
  msg[0] = dst;

  while (loop != MSG_END)
    {
      while (1)
	{
	  pt = fgets (string, 256, fp);
	  if (pt[0] == '#' || pt[0] == '\r' || pt[0] == '\n')
	    continue;
	  if (pt != NULL)
	    break;
	  else
	    {
	      ret = -3;
	      goto load_language_msg_error;
	    }
	}

      if (!strncmp (pt, end, cmplen - 2))
	break;


      len = strlen (pt);
      // memcpy(dst, pt, len);
      // Replace key definitions (*letter) with Pictochat icons
      // while copying.
      unsigned int srcChar, dstLen = 0;
      for (srcChar = 0; srcChar < len; srcChar++)
	{
	  if (pt[srcChar] == '*')
	    {
	      switch (pt[srcChar + 1])
		{
		case 'A':
		  memcpy (&dst[dstLen], HOTKEY_A_DISPLAY,
			  sizeof (HOTKEY_A_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_A_DISPLAY) - 1;
		  break;
		case 'B':
		  memcpy (&dst[dstLen], HOTKEY_B_DISPLAY,
			  sizeof (HOTKEY_B_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_B_DISPLAY) - 1;
		  break;
		case 'X':
		  memcpy (&dst[dstLen], HOTKEY_X_DISPLAY,
			  sizeof (HOTKEY_X_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_X_DISPLAY) - 1;
		  break;
		case 'Y':
		  memcpy (&dst[dstLen], HOTKEY_Y_DISPLAY,
			  sizeof (HOTKEY_Y_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_Y_DISPLAY) - 1;
		  break;
		case 'L':
		  memcpy (&dst[dstLen], HOTKEY_L_DISPLAY,
			  sizeof (HOTKEY_L_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_L_DISPLAY) - 1;
		  break;
		case 'R':
		  memcpy (&dst[dstLen], HOTKEY_R_DISPLAY,
			  sizeof (HOTKEY_R_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_R_DISPLAY) - 1;
		  break;
		case 'S':
		  memcpy (&dst[dstLen], HOTKEY_START_DISPLAY,
			  sizeof (HOTKEY_START_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_START_DISPLAY) - 1;
		  break;
		case 's':
		  memcpy (&dst[dstLen], HOTKEY_SELECT_DISPLAY,
			  sizeof (HOTKEY_SELECT_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_SELECT_DISPLAY) - 1;
		  break;
		case 'u':
		  memcpy (&dst[dstLen], HOTKEY_UP_DISPLAY,
			  sizeof (HOTKEY_UP_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_UP_DISPLAY) - 1;
		  break;
		case 'd':
		  memcpy (&dst[dstLen], HOTKEY_DOWN_DISPLAY,
			  sizeof (HOTKEY_DOWN_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_DOWN_DISPLAY) - 1;
		  break;
		case 'l':
		  memcpy (&dst[dstLen], HOTKEY_LEFT_DISPLAY,
			  sizeof (HOTKEY_LEFT_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_LEFT_DISPLAY) - 1;
		  break;
		case 'r':
		  memcpy (&dst[dstLen], HOTKEY_RIGHT_DISPLAY,
			  sizeof (HOTKEY_RIGHT_DISPLAY) - 1);
		  srcChar++;
		  dstLen += sizeof (HOTKEY_RIGHT_DISPLAY) - 1;
		  break;
		case '\0':
		  dst[dstLen] = pt[srcChar];
		  dstLen++;
		  break;
		default:
		  memcpy (&dst[dstLen], &pt[srcChar], 2);
		  srcChar++;
		  dstLen += 2;
		  break;
		}
	    }
	  else
	    {
	      dst[dstLen] = pt[srcChar];
	      dstLen++;
	    }
	}

      dst += dstLen;
      //at a line return, when "\n" paded, this message not end
      if (*(dst - 1) == 0x0A)
	{
	  pt = strrchr (pt, '\\');
	  if ((pt != NULL) && (*(pt + 1) == 'n'))
	    {
	      if (*(dst - 2) == 0x0D)
		{
		  *(dst - 4) = '\n';
		  dst -= 3;
		}
	      else
		{
		  *(dst - 3) = '\n';
		  dst -= 2;
		}
	    }
	  else			//a message end
	    {
	      if (*(dst - 2) == 0x0D)
		dst -= 1;
	      *(dst - 1) = '\0';
	      msg[++loop] = dst;
	    }
	}
    }

#if 0
  loop = 0;
  printf ("------\n");
  while (loop != MSG_END)
    printf ("%d: %s\n", loop, msg[loop++]);
#endif

load_language_msg_error:
  fclose (fp);
  return ret;
}
