#pragma once

/**
 * Headers for the C only interface.
 */


/**
 * @brief Check whether the author of an AppImage doesn't want it to be integrated.
 *
 * An AppImage is considered that shall not be integrated if fulfill any of the following conditions:
 *  - The AppImage's desktop file has set X-AppImage-Integrate=false.
 *  - The AppImage's desktop file has set Terminal=true.
 *
 * @param appImagePath
 * @return true if any of the conditions are meet, false otherwise
 */
bool appimage_shall_not_be_integrated(const char* path);
