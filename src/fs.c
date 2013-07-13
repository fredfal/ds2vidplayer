#include <string.h>

#include "ffReader_config.h"
#include "fs.h"
#include "filetypes_config.h"

const char *
fs_get_fileextension (const char *filename)
{
  size_t len = strlen (filename);
  const char *position;

  position = filename + len;

  while (position > filename && *position != CONFIG_FILEEXTENSION_SEPARATOR
	 && *position != CONFIG_DIR_SEPARATOR)
    position--;

  if (*position == '.')
    return position + 1;
  else
    return NULL;
}

const char *
fs_basename (const char *pathname)
{
  const char *basename = strrchr (pathname, CONFIG_DIR_SEPARATOR);
  if (basename == NULL)
    basename = pathname;
  else
    basename++;

  return basename;
}

t_fs_filetype_e
fs_get_filetype (const char *filename)
{
  const char *extension = fs_get_fileextension (filename);
  t_fs_filetype_entry *filetype_entry;
  ft_table;
  t_fs_specfiletype_entry *specfiletype_entry;

  filetype_entry = ft_table;
  specfiletype_entry = ft_spec_table;

  if (extension)
    {
      while (filetype_entry->extension != NULL)
	{
	  if (strcasecmp (extension, filetype_entry->extension) == 0)
	    {
	      return filetype_entry->filetype;
	    }
	  filetype_entry++;
	}
    }

  while (specfiletype_entry->filename != NULL)
    {
      if (strcasecmp (fs_basename (filename), specfiletype_entry->filename) ==
	  0)
	{
	  return specfiletype_entry->filetype;
	}
      specfiletype_entry++;
    }

  return fs_filetype_unknown;
}

t_fs_imagefile_spec_entry *
get_imagefile_spec (const char *filename)
{
  t_fs_imagefile_spec_entry *imagefile_spec;
  t_fs_filetype_e filetype;
  int i;

  filetype = fs_get_filetype (filename);

  for (i = 0; i < FS_IMAGEFILE_SPEC_TABLE_SIZE; i++)
    {
      if (filetype == fs_imagefile_spec_table[i].filetype)
	{
	  imagefile_spec = &fs_imagefile_spec_table[i];
	  break;
	}
    }

  return imagefile_spec;
}

t_fs_archivefile_spec_entry *
get_archivefile_spec (const char *filename)
{
  t_fs_archivefile_spec_entry *archivefile_spec;
  t_fs_filetype_e filetype;
  int i;

  filetype = fs_get_filetype (filename);

  for (i = 0; i < FS_ARCHIVEFILE_SPEC_TABLE_SIZE; i++)
    {
      if (filetype == fs_archivefile_spec_table[i].filetype)
	{
	  archivefile_spec = &fs_archivefile_spec_table[i];
	  break;
	}
    }

  return archivefile_spec;
}
