#include "filetypes_config.h"
#include "image.h"
#include "bitmap.h"
#include "archive.h"

t_fs_filetype_entry ft_table[] =
{
        {"lrc", fs_filetype_txt},
        {"txt", fs_filetype_txt},
        {"log", fs_filetype_txt},
        {"ini", fs_filetype_txt},
        {"cfg", fs_filetype_txt},
        {"conf", fs_filetype_txt},
        {"inf", fs_filetype_txt},
        {"xml", fs_filetype_txt},
        {"cpp", fs_filetype_txt},
        {"in", fs_filetype_txt},
        {"am", fs_filetype_txt},
        {"mak", fs_filetype_txt},
        {"exp", fs_filetype_txt},
        {"sh", fs_filetype_txt},
        {"asm", fs_filetype_txt},
        {"s", fs_filetype_txt},
        {"patch", fs_filetype_txt},
        {"c", fs_filetype_txt},
        {"h", fs_filetype_txt},
        {"hpp", fs_filetype_txt},
        {"cc", fs_filetype_txt},
        {"cxx", fs_filetype_txt},
        {"pas", fs_filetype_txt},
        {"bas", fs_filetype_txt},
        {"py", fs_filetype_txt},
        {"mk", fs_filetype_txt},
        {"rc", fs_filetype_txt},
        {"pl", fs_filetype_txt},
        {"cgi", fs_filetype_txt},
        {"bat", fs_filetype_txt},
        {"js", fs_filetype_txt},
        {"vbs", fs_filetype_txt},
        {"vb", fs_filetype_txt},
        {"cs", fs_filetype_txt},
        {"css", fs_filetype_txt},
        {"csv", fs_filetype_txt},
        {"php", fs_filetype_txt},
        {"php3", fs_filetype_txt},
        {"asp", fs_filetype_txt},
        {"aspx", fs_filetype_txt},
        {"java", fs_filetype_txt},
        {"jsp", fs_filetype_txt},
        {"awk", fs_filetype_txt},
        {"tcl", fs_filetype_txt},
        {"y", fs_filetype_txt},
        {"html", fs_filetype_html},
        {"htm", fs_filetype_html},
        {"shtml", fs_filetype_html},
#ifdef ENABLE_IMAGE
        {"png", fs_filetype_png},
        {"gif", fs_filetype_gif},
        {"jpg", fs_filetype_jpg},
        {"jpeg", fs_filetype_jpg},
        {"bmp", fs_filetype_bmp},
        {"tif", fs_filetype_tif},
#endif
        {"zip", fs_filetype_zip},
        {"gz", fs_filetype_gz},
        {"chm", fs_filetype_chm},
        {"rar", fs_filetype_rar},
        {"pbp", fs_filetype_prog},
        {"ebm", fs_filetype_ebm},
#ifdef ENABLE_TTF
        {"ttf", fs_filetype_font},
        {"ttc", fs_filetype_font},
#endif
#if ENABLE_PDF
        {"pdf", fs_filetype_pdf},
#endif
        {NULL, fs_filetype_unknown}
};

t_fs_specfiletype_entry ft_spec_table[] = {
        {"Makefile", fs_filetype_txt},
        {"LICENSE", fs_filetype_txt},
        {"TODO", fs_filetype_txt},
        {"Configure", fs_filetype_txt},
        {"Changelog", fs_filetype_txt},
        {"Readme", fs_filetype_txt},
        {"Version", fs_filetype_txt},
        {"INSTALL", fs_filetype_txt},
        {"CREDITS", fs_filetype_txt},
        {NULL, fs_filetype_unknown}
};

t_fs_filetype_e fs_archive_class[] = {fs_filetype_zip,fs_filetype_rar};

t_fs_archivefile_spec_entry fs_archivefile_spec_table[1] = {
	{fs_filetype_zip, "ZIP Archive", &zip_fopen, &zip_fclose, &zip_fread, &zip_fseek}
};

t_fs_imagefile_spec_entry fs_imagefile_spec_table[1] = {
	{fs_filetype_bmp, "Microsoft Bitmap Image", &BMP_read},
};
