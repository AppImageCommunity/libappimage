#pragma once

// local includes
#include "appimage_handler.h"

appimage_handler appimage_type_1_create_handler();

/**
 * According to the AppImage specification for type 1 files
 * https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-1-image-format
 *
 * Match the buffer to 0x414901.
 * @param buffer
 * @return 1 if the values are the same, 0 otherwise
 */
bool match_type_1_magic_bytes(const char* buffer);

/**
 * Check for iso 9660 magic bytes.
 *
 * According to the AppImage specification for type 1 files
 * https://github.com/AppImage/AppImageSpec/blob/master/draft.md#type-1-image-format
 * the files must be valid ISO 9660 files.
 *
 * @param path path
 * @return true if the file has the proper signature, false otherwise
 */
bool is_iso_9660_file(const char* path);