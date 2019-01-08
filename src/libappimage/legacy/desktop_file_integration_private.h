#pragma once

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* Finds a file with '.Desktop' or '.desktop' suffix.
* @param path
* @return newly allocated const char* with the full path of the file or NULL. Result must be freed.
*/
char* find_desktop_file(const char* appdir_path);

GKeyFile* load_desktop_file(const char* file_path);

#ifdef __cplusplus
}
#endif
