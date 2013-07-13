#ifndef _FS_H_
#define _FS_H_

#include "ffReader_config.h"
#include "filetypes_config.h"

#define CONFIG_DIR_SEPARATOR '/'
#define CONFIG_FILEEXTENSION_SEPARATOR '.'

t_fs_filetype_e fs_get_filetype (const char *filename);
t_fs_imagefile_spec_entry *get_imagefile_spec (const char *filename);
t_fs_archivefile_spec_entry *get_archivefile_spec (const char *filename);

#endif
