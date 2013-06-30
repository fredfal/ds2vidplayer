#ifndef _FILETYPES_CONFIG_H_
#define _FILETYPES_CONFIG_H_

#include "stdio.h"
#include "image.h"
#include "ffReader_config.h"

typedef enum
{
        fs_filetype_dir = 0,
        fs_filetype_chm,
        fs_filetype_gz,
        fs_filetype_zip,
        fs_filetype_rar,
        fs_filetype_txt,
        fs_filetype_html,
        fs_filetype_prog,
#ifdef ENABLE_IMAGE
        fs_filetype_bmp,
        fs_filetype_tif,
        fs_filetype_gif,
        fs_filetype_jpg,
        fs_filetype_png,
#endif
        fs_filetype_ebm,
#ifdef ENABLE_TTF
        fs_filetype_font,
#endif
#if ENABLE_PDF
        fs_filetype_pdf,
#endif
        fs_filetype_unknown
} t_fs_filetype_e;

typedef enum
{
	fs_fileclass_archive,
	fs_fileclass_image,
} t_fs_fileclass_e;

typedef struct
{
        const char *extension;
        t_fs_filetype_e filetype;
} t_fs_filetype_entry;

typedef struct
{
        const char *filename;
        t_fs_filetype_e filetype;
} t_fs_specfiletype_entry;

typedef struct
{
	t_fs_filetype_e filetype;
	const char* long_name;
        void *(*fopen_function)(const char *archive_filename, const char *filename, const char *mode);
        void (*fclose_function)(void *stream);
	size_t (*fread_function)(void *ptr, size_t size, size_t nmemb, void *stream);
	int (*fseek_function)(void *stream, long offset, int whence);
} t_fs_archivefile_spec_entry;

typedef struct
{
	t_fs_filetype_e filetype;
        const char* long_name;
	image_data_s *(*load_image)(void* fp, int max_width, int max_height,
                                    size_t (*fread_fonction)(void *ptr, size_t size, size_t nmemb, void *stream),
                                    int (*fseek_function)(void *stream, long offset, int whence),
                                    image_data_s *image_data);
} t_fs_imagefile_spec_entry;

t_fs_filetype_entry ft_table[];
t_fs_specfiletype_entry ft_spec_table[];
t_fs_filetype_e fs_archive_class[];
#define FS_ARCHIVEFILE_SPEC_TABLE_SIZE (sizeof(fs_archivefile_spec_table)/sizeof(t_fs_archivefile_spec_entry))
t_fs_archivefile_spec_entry fs_archivefile_spec_table[1];

#define FS_IMAGEFILE_SPEC_TABLE_SIZE (sizeof(fs_imagefile_spec_table)/sizeof(t_fs_imagefile_spec_entry))
t_fs_imagefile_spec_entry fs_imagefile_spec_table[1];

t_fs_filetype_e fs_get_filetype(const char *filename);

#endif
