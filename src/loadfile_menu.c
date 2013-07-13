#include "common.h"
#include "gui_action.h"
#include "draw.h"
#include "gui.h"
#include "message.h"
#include "ds2io.h"
#include "ds2_malloc.h"

/*--------------------------------------------------------
	Sort function
--------------------------------------------------------*/
static int
nameSortFunction (char *a, char *b)
{
  // ".." sorts before everything except itself.
  bool aIsParent = strcmp (a, "..") == 0;
  bool bIsParent = strcmp (b, "..") == 0;

  if (aIsParent && bIsParent)
    return 0;
  else if (aIsParent)		// Sorts before
    return -1;
  else if (bIsParent)		// Sorts after
    return 1;
  else
    return strcasecmp (a, b);
}

/*
 * Determines whether a portion of a vector is sorted.
 * Input assertions: 'from' and 'to' are valid indices into data. 'to' can be
 *   the maximum value for the type 'unsigned int'.
 * Input: 'data', data vector, possibly sorted.
 *        'sortFunction', function determining the sort order of two elements.
 *        'from', index of the first element in the range to test.
 *        'to', index of the last element in the range to test.
 * Output: true if, for any valid index 'i' such as from <= i < to,
 *   data[i] < data[i + 1].
 *   true if the range is one or no elements, or if from > to.
 *   false otherwise.
 */
static bool
isSorted (char **data, int (*sortFunction) (char *, char *),
	  unsigned int from, unsigned int to)
{
  if (from >= to)
    return true;

  char **prev = &data[from];
  unsigned int i;
  for (i = from + 1; i < to; i++)
    {
      if ((*sortFunction) (*prev, data[i]) > 0)
	return false;
      prev = &data[i];
    }
  if ((*sortFunction) (*prev, data[to]) > 0)
    return false;

  return true;
}

/*
 * Chooses a pivot for Quicksort. Uses the median-of-three search algorithm
 * first proposed by Robert Sedgewick.
 * Input assertions: 'from' and 'to' are valid indices into data. 'to' can be
 *   the maximum value for the type 'unsigned int'.
 * Input: 'data', data vector.
 *        'sortFunction', function determining the sort order of two elements.
 *        'from', index of the first element in the range to be sorted.
 *        'to', index of the last element in the range to be sorted.
 * Output: a valid index into data, between 'from' and 'to' inclusive.
 */
static unsigned int
choosePivot (char **data, int (*sortFunction) (char *, char *),
	     unsigned int from, unsigned int to)
{
  // The highest of the two extremities is calculated first.
  unsigned int highest = ((*sortFunction) (data[from], data[to]) > 0)
    ? from : to;
  // Then the lowest of that highest extremity and the middle
  // becomes the pivot.
  return ((*sortFunction) (data[from + (to - from) / 2], data[highest]) <
	  0) ? (from + (to - from) / 2) : highest;
}

/*
 * Partition function for Quicksort. Moves elements such that those that are
 * less than the pivot end up before it in the data vector.
 * Input assertions: 'from', 'to' and 'pivotIndex' are valid indices into data.
 *   'to' can be the maximum value for the type 'unsigned int'.
 * Input: 'data', data vector.
 *        'metadata', data describing the values in 'data'.
 *        'sortFunction', function determining the sort order of two elements.
 *        'from', index of the first element in the range to sort.
 *        'to', index of the last element in the range to sort.
 *        'pivotIndex', index of the value chosen as the pivot.
 * Output: the index of the value chosen as the pivot after it has been moved
 *   after all the values that are less than it.
 */
static unsigned int
partition (char **data, u8 * metadata, int (*sortFunction) (char *, char *),
	   unsigned int from, unsigned int to, unsigned int pivotIndex)
{
  char *pivotValue = data[pivotIndex];
  data[pivotIndex] = data[to];
  data[to] = pivotValue;
  {
    u8 tM = metadata[pivotIndex];
    metadata[pivotIndex] = metadata[to];
    metadata[to] = tM;
  }

  unsigned int storeIndex = from;
  unsigned int i;
  for (i = from; i < to; i++)
    {
      if ((*sortFunction) (data[i], pivotValue) < 0)
	{
	  char *tD = data[storeIndex];
	  data[storeIndex] = data[i];
	  data[i] = tD;
	  u8 tM = metadata[storeIndex];
	  metadata[storeIndex] = metadata[i];
	  metadata[i] = tM;
	  ++storeIndex;
	}
    }

  {
    char *tD = data[to];
    data[to] = data[storeIndex];
    data[storeIndex] = tD;
    u8 tM = metadata[to];
    metadata[to] = metadata[storeIndex];
    metadata[storeIndex] = tM;
  }
  return storeIndex;
}

/*
 * Sorts an array while keeping metadata in sync.
 * This sort is unstable and its average performance is
 *   O(data.size() * log2(data.size()).
 * Input assertions: for any valid index 'i' in data, index 'i' is valid in
 *   metadata. 'from' and 'to' are valid indices into data. 'to' can be
 *   the maximum value for the type 'unsigned int'.
 * Invariant: index 'i' in metadata describes index 'i' in data.
 * Input: 'data', data to sort.
 *        'metadata', data describing the values in 'data'.
 *        'sortFunction', function determining the sort order of two elements.
 *        'from', index of the first element in the range to sort.
 *        'to', index of the last element in the range to sort.
 */
static void
quickSort (char **data, u8 * metadata, int (*sortFunction) (char *, char *),
	   unsigned int from, unsigned int to)
{
  if (isSorted (data, sortFunction, from, to))
    return;

  unsigned int pivotIndex = choosePivot (data, sortFunction, from, to);
  unsigned int newPivotIndex =
    partition (data, metadata, sortFunction, from, to, pivotIndex);
  if (newPivotIndex > 0)
    quickSort (data, metadata, sortFunction, from, newPivotIndex - 1);
  if (newPivotIndex < to)
    quickSort (data, metadata, sortFunction, newPivotIndex + 1, to);
}

static void
strupr (char *str)
{
  while (*str)
    {
      if (*str <= 0x7A && *str >= 0x61)
	*str -= 0x20;
      str++;
    }
}

// ******************************************************************************
// get file list
// ******************************************************************************

s32
load_file (char **wildcards, char *result, char *default_dir_name)
{
  if (default_dir_name == NULL || *default_dir_name == '\0')
    return -4;

  char CurrentDirectory[MAX_PATH];
  u32 ContinueDirectoryRead = 1;
  u32 ReturnValue;
  u32 i;

  //strcpy (CurrentDirectory, default_dir_name);
  strcpy (CurrentDirectory, "fat:");

  printf("%s\n", CurrentDirectory);

  while (ContinueDirectoryRead)
    {
      printf("%d\n", ContinueDirectoryRead);
      // Read the current directory. This loop is continued every time
      // the
      // current directory changes.
      ds2_setCPUclocklevel (13);

      show_icon (down_screen_addr, &ICON_SUBBG, 0, 0);
      show_icon (down_screen_addr, &ICON_TITLE, 0, 0);
      show_icon (down_screen_addr, &ICON_TITLEICON, TITLE_ICON_X,
		 TITLE_ICON_Y);
      PRINT_STRING_BG (down_screen_addr,
		       msg[MSG_FILE_MENU_LOADING_LIST], COLOR_WHITE,
		       COLOR_TRANS, 49, 10);
      ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

      u32 LastCountDisplayTime = getSysTime ();

      char **EntryNames = NULL;
      u8 *EntryDirectoryFlags = NULL;
      DIR *CurrentDirHandle = NULL;
      u32 EntryCount = 1, EntryCapacity = 4 /* initial */ ;

      EntryNames = (char **) malloc (EntryCapacity * sizeof (char *));
      if (EntryNames == NULL)
	{
	  ReturnValue = -2;
	  ContinueDirectoryRead = 0;
	  goto cleanup;
	}

      EntryDirectoryFlags = (u8 *) malloc (EntryCapacity * sizeof (u8));
      if (EntryDirectoryFlags == NULL)
	{
	  ReturnValue = -2;
	  ContinueDirectoryRead = 0;
	  goto cleanup;
	}

      CurrentDirHandle = opendir (CurrentDirectory);
      if (CurrentDirHandle == NULL)
	{
	  ReturnValue = -1;
	  ContinueDirectoryRead = 0;
          printf("Error opening directory %s\n", CurrentDirectory);
	  goto cleanup;
	}

      EntryNames[0] = "..";
      EntryDirectoryFlags[0] = 1;

      dirent *CurrentEntryHandle;
      struct stat Stat;

      while ((CurrentEntryHandle =
	      readdir_ex (CurrentDirHandle, &Stat)) != NULL)
	{
	  u32 Now = getSysTime ();
	  u32 AddEntry = 0;
	  char *Name = CurrentEntryHandle->d_name;

	  if (Now >= LastCountDisplayTime + 5859 /* 250 ms */ )
	    {
	      LastCountDisplayTime = Now;

	      show_icon (down_screen_addr, &ICON_TITLE, 0, 0);
	      show_icon (down_screen_addr, &ICON_TITLEICON,
			 TITLE_ICON_X, TITLE_ICON_Y);
	      char Line[384], Element[128];
	      strcpy (Line, msg[MSG_FILE_MENU_LOADING_LIST]);
	      sprintf (Element, " (%u)", EntryCount);
	      strcat (Line, Element);
	      PRINT_STRING_BG (down_screen_addr, Line,
			       COLOR_WHITE, COLOR_TRANS, 49, 10);
	      ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);
	    }

	  if (S_ISDIR (Stat.st_mode))
	    {
	      // Add directories no matter what, except for the special
	      // ones, "." and "..".
	      if (!(Name[0] == '.' &&
		    (Name[1] == '\0' || (Name[1] == '.' && Name[2] == '\0'))))
		{
		  AddEntry = 1;
		}
	    }
	  else
	    {
	      if (wildcards[0] == NULL)	// Show every file
		AddEntry = 1;
	      else
		{
		  // Add files only if their extension is in the list.
		  char *Extension = strrchr (Name, '.');
		  if (Extension != NULL)
		    {
		      for (i = 0; wildcards[i] != NULL; i++)
			{
			  if (strcasecmp (Extension, wildcards[i]) == 0)
			    {
			      AddEntry = 1;
			      break;
			    }
			}
		    }
		}
	    }

	  if (AddEntry)
	    {
	      // Ensure we have enough capacity in the char* array
	      // first.
	      if (EntryCount == EntryCapacity)
		{
		  u32 NewCapacity = EntryCapacity * 2;
		  void *NewEntryNames = realloc (EntryNames,
						 NewCapacity *
						 sizeof (char *));
		  if (NewEntryNames == NULL)
		    {
		      ReturnValue = -2;
		      ContinueDirectoryRead = 0;
		      goto cleanup;
		    }
		  else
		    EntryNames = NewEntryNames;

		  void *NewEntryDirectoryFlags = realloc (EntryDirectoryFlags,
							  NewCapacity *
							  sizeof (u8));
		  if (NewEntryDirectoryFlags == NULL)
		    {
		      ReturnValue = -2;
		      ContinueDirectoryRead = 0;
		      goto cleanup;
		    }
		  else
		    EntryDirectoryFlags = NewEntryDirectoryFlags;

		  EntryCapacity = NewCapacity;
		}
	      // Then add the entry.
	      EntryNames[EntryCount] = malloc (strlen (Name) + 1);
	      if (EntryNames[EntryCount] == NULL)
		{
		  ReturnValue = -2;
		  ContinueDirectoryRead = 0;
		  goto cleanup;
		}

	      strcpy (EntryNames[EntryCount], Name);
	      if (S_ISDIR (Stat.st_mode))
		EntryDirectoryFlags[EntryCount] = 1;
	      else
		EntryDirectoryFlags[EntryCount] = 0;

	      EntryCount++;
	    }
	}

      show_icon (down_screen_addr, &ICON_TITLE, 0, 0);
      show_icon (down_screen_addr, &ICON_TITLEICON, TITLE_ICON_X,
		 TITLE_ICON_Y);
      PRINT_STRING_BG (down_screen_addr,
		       msg[MSG_FILE_MENU_SORTING_LIST], COLOR_WHITE,
		       COLOR_TRANS, 49, 10);
      ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

      quickSort (EntryNames, EntryDirectoryFlags, nameSortFunction, 1,
		 EntryCount - 1);
      ds2_setCPUclocklevel (0);

      u32 ContinueInput = 1;
      s32 SelectedIndex = 0;
      u32 DirectoryScrollDirection = 0x8000;	// First
      // scroll
      // to the
      // left
      s32 EntryScrollValue = 0;
      u32 ModifyScrollers = 1;
      u32 ScrollerCount = 0;

      draw_hscroll_init (down_screen_addr, 49, 10, 199, COLOR_TRANS,
			 COLOR_WHITE, CurrentDirectory);
      ScrollerCount++;

      // Show the current directory and get input. This loop is
      // continued
      // every frame, because the current directory scrolls atop the
      // screen.

      while (ContinueDirectoryRead && ContinueInput)
	{
	  // Try to use a row set such that the selected entry is in the
	  // middle of the screen.
	  s32 LastEntry = SelectedIndex + FILE_LIST_ROWS / 2;

	  // If the last row is out of bounds, put it back in bounds.
	  // (In this case, the user has selected an entry in the last
	  // FILE_LIST_ROWS / 2.)
	  if (LastEntry >= EntryCount)
	    LastEntry = EntryCount - 1;

	  s32 FirstEntry = LastEntry - (FILE_LIST_ROWS - 1);

	  // If the first row is out of bounds, put it back in bounds.
	  // (In this case, the user has selected an entry in the first
	  // FILE_LIST_ROWS / 2, or there are fewer than FILE_LIST_ROWS
	  // entries.)
	  if (FirstEntry < 0)
	    {
	      FirstEntry = 0;

	      // If there are more than FILE_LIST_ROWS / 2 files,
	      // we need to enlarge the first page.
	      LastEntry = FILE_LIST_ROWS - 1;
	      if (LastEntry >= EntryCount)	// No...
		LastEntry = EntryCount - 1;
	    }
	  // Update scrollers.
	  // a) If a different item has been selected, remake entry
	  // scrollers, resetting the formerly selected entry to the
	  // start and updating the selection color.
	  if (ModifyScrollers)
	    {
	      // Preserve the directory scroller.
	      for (; ScrollerCount > 1; ScrollerCount--)
		draw_hscroll_over (ScrollerCount - 1);
	      for (i = FirstEntry; i <= LastEntry; i++)
		{
		  u16 color = (SelectedIndex == i)
		    ? COLOR_ACTIVE_ITEM : COLOR_INACTIVE_ITEM;
		  if (hscroll_init
		      (down_screen_addr,
		       FILE_SELECTOR_NAME_X,
		       GUI_ROW1_Y + (i -
				     FirstEntry) *
		       GUI_ROW_SY + TEXT_OFFSET_Y,
		       FILE_SELECTOR_NAME_SX, COLOR_TRANS,
		       color, EntryNames[i]) < 0)
		    {
		      ReturnValue = -2;
		      ContinueDirectoryRead = 0;
		      goto cleanupScrollers;
		    }
		  else
		    {
		      ScrollerCount++;
		    }
		}

	      ModifyScrollers = 0;
	    }
	  // b) Must we update the directory scroller?
	  if ((DirectoryScrollDirection & 0xFF) >= 0x20)
	    {
	      if (DirectoryScrollDirection & 0x8000)	// scroll left
		{
		  if (draw_hscroll (0, -1) == 0)
		    DirectoryScrollDirection = 0;	// scroll right
		}
	      else
		{
		  if (draw_hscroll (0, 1) == 0)
		    DirectoryScrollDirection = 0x8000;	// scroll
		  // left
		}
	    }
	  else
	    {
	      // Wait one less frame before scrolling the directory
	      // again.
	      DirectoryScrollDirection++;
	    }

	  // c) Must we scroll the current file as a result of user
	  // input?
	  if (EntryScrollValue != 0)
	    {
	      draw_hscroll (SelectedIndex - FirstEntry + 1, EntryScrollValue);
	      EntryScrollValue = 0;
	    }
	  // Draw.
	  // a) The background.
	  show_icon (down_screen_addr, &ICON_SUBBG, 0, 0);
	  show_icon (down_screen_addr, &ICON_TITLE, 0, 0);
	  show_icon (down_screen_addr, &ICON_TITLEICON,
		     TITLE_ICON_X, TITLE_ICON_Y);

	  // b) The selection background.
	  show_icon (down_screen_addr, &ICON_SUBSELA, SUBSELA_X,
		     GUI_ROW1_Y + (SelectedIndex -
				   FirstEntry) * GUI_ROW_SY +
		     SUBSELA_OFFSET_Y);

	  // c) The scrollers.
	  for (i = 0; i < ScrollerCount; i++)
	    draw_hscroll (i, 0);

	  // d) The icons.
	  for (i = FirstEntry; i <= LastEntry; i++)
	    {
	      struct gui_iconlist *icon;
	      if (i == 0)
		icon = &ICON_DOTDIR;
	      else if (EntryDirectoryFlags[i])
		icon = &ICON_DIRECTORY;
	      else
		{
		  char *Extension = strrchr (EntryNames[i], '.');
		  if (Extension != NULL)
		    {
		      if (strcasecmp
			  (Extension, ".zip") == 0
			  || strcasecmp (Extension, ".gz") == 0)
			icon = &ICON_ZIPFILE;
		      else
			icon = &ICON_UNKNOW;
		    }
		  else
		    icon = &ICON_UNKNOW;
		}

	      show_icon (down_screen_addr, icon,
			 FILE_SELECTOR_ICON_X,
			 GUI_ROW1_Y + (i -
				       FirstEntry) *
			 GUI_ROW_SY + FILE_SELECTOR_ICON_Y);
	    }

	  ds2_flipScreen (DOWN_SCREEN, DOWN_SCREEN_UPDATE_METHOD);

	  // Delay before getting the input.
	  mdelay (20);

	  struct key_buf inputdata;
	  gui_action_type gui_action = get_gui_input ();
	  ds2_getrawInput (&inputdata);

	  // Get KEY_RIGHT and KEY_LEFT separately to allow scrolling
	  // the selected file name faster.
	  if (inputdata.key & KEY_RIGHT)
	    EntryScrollValue = -3;
	  else if (inputdata.key & KEY_LEFT)
	    EntryScrollValue = 3;

	  switch (gui_action)
	    {
	    case CURSOR_TOUCH:
	      {
		wait_Allkey_release (0);
		// ___ 33 This screen has 6 possible rows. Touches
		// ___ 60 above or below these are ignored.
		// . . . (+27)
		// ___ 192
		if (inputdata.y <= GUI_ROW1_Y
		    || inputdata.y > NDS_SCREEN_HEIGHT)
		  break;

		u32 mod = (inputdata.y - GUI_ROW1_Y) / GUI_ROW_SY;

		if (mod >= LastEntry - FirstEntry + 1)
		  break;

		SelectedIndex = FirstEntry + mod;
		/*
		 * fall through 
		 */
	      }

	    case CURSOR_SELECT:
	      wait_Allkey_release (0);
	      if (SelectedIndex == 0)	// The parent directory
		{
		  char *SlashPos = strrchr (CurrentDirectory, '/');
		  if (SlashPos != NULL)	// There's a parent
		    {
		      *SlashPos = '\0';
		      ContinueInput = 0;
		    }
		  else		// We're at the root
		    {
		      ReturnValue = -1;
		      ContinueDirectoryRead = 0;
		    }
		}
	      else if (EntryDirectoryFlags[SelectedIndex])
		{
		  strcat (CurrentDirectory, "/");
		  strcat (CurrentDirectory, EntryNames[SelectedIndex]);
		  ContinueInput = 0;
		}
	      else
		{
		  strcpy (default_dir_name, CurrentDirectory);
		  strcpy (result, EntryNames[SelectedIndex]);
		  ReturnValue = 0;
		  ContinueDirectoryRead = 0;
		}
	      break;

	    case CURSOR_UP:
	      SelectedIndex--;
	      if (SelectedIndex < 0)
		SelectedIndex++;
	      else
		ModifyScrollers = 1;
	      break;

	    case CURSOR_DOWN:
	      SelectedIndex++;
	      if (SelectedIndex >= EntryCount)
		SelectedIndex--;
	      else
		ModifyScrollers = 1;
	      break;

	      // scroll page down
	    case CURSOR_RTRIGGER:
	      {
		u32 OldIndex = SelectedIndex;
		SelectedIndex += FILE_LIST_ROWS;
		if (SelectedIndex >= EntryCount)
		  SelectedIndex = EntryCount - 1;
		if (SelectedIndex != OldIndex)
		  ModifyScrollers = 1;
		break;
	      }

	      // scroll page up
	    case CURSOR_LTRIGGER:
	      {
		u32 OldIndex = SelectedIndex;
		SelectedIndex -= FILE_LIST_ROWS;
		if (SelectedIndex < 0)
		  SelectedIndex = 0;
		if (SelectedIndex != OldIndex)
		  ModifyScrollers = 1;
		break;
	      }

	    case CURSOR_BACK:
	      {
		wait_Allkey_release (0);
		char *SlashPos = strrchr (CurrentDirectory, '/');
		if (SlashPos != NULL)	// There's a parent
		  {
		    *SlashPos = '\0';
		    ContinueInput = 0;
		  }
		else		// We're at the root
		  {
		    ReturnValue = -1;
		    ContinueDirectoryRead = 0;
		  }
		break;
	      }

	    case CURSOR_EXIT:
	      wait_Allkey_release (0);
	      ReturnValue = -1;
	      ContinueDirectoryRead = 0;
	      break;

	    default:
	      break;
	    }			// end switch
	}			// end while

    cleanupScrollers:
      for (; ScrollerCount > 0; ScrollerCount--)
	draw_hscroll_over (ScrollerCount - 1);

    cleanup:
      if (CurrentDirHandle != NULL)
	closedir (CurrentDirHandle);

      if (EntryDirectoryFlags != NULL)
	free (EntryDirectoryFlags);
      if (EntryNames != NULL)
	{
	  // EntryNames[0] is "..", a literal. Don't free it.
	  for (; EntryCount > 1; EntryCount--)
	    free (EntryNames[EntryCount - 1]);
	  free (EntryNames);
	}
    }				// end while

  return ReturnValue;
}
