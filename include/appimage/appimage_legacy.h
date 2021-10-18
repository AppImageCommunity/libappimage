#pragma once

// system
#include <stdio.h>

// local
#include <appimage/config.h>

/**
 * All of the functions in this header are deprecated and must not be used in newly written code
 */

/*
 * Checks whether a type 1 AppImage's desktop file has set Terminal=true.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type1_is_terminal_app(const char* path) __attribute__ ((deprecated));

/*
 * Checks whether a type 2 AppImage's desktop file has set Terminal=true.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type2_is_terminal_app(const char* path) __attribute__ ((deprecated));

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
/*
 * Checks whether a type 1 AppImage's desktop file has set X-AppImage-Version=false.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type1_shall_not_be_integrated(const char* path) __attribute__ ((deprecated));

/*
 * Checks whether a type 2 AppImage's desktop file has set X-AppImage-Version=false.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type2_shall_not_be_integrated(const char* path) __attribute__ ((deprecated));

/* Register a type 1 AppImage in the system
 * DEPRECATED don't use in newly written code. Use appimage_is_registered_in_system instead.
 * */
bool appimage_type1_register_in_system(const char* path, bool verbose) __attribute__ ((deprecated));

/* Register a type 2 AppImage in the system
 * DEPRECATED don't use in newly written code. Use appimage_is_registered_in_system instead.
 * */
bool appimage_type2_register_in_system(const char* path, bool verbose) __attribute__ ((deprecated));

#endif // LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
