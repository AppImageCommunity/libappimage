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

