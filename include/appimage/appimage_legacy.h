#pragma once

// system
#include <stdio.h>

/**
 * All of the functions in this header are deprecated and must not be used in newly written code
 */

/*
 * Calculate the size of an ELF file on disk based on the information in its header
 *
 * Example:
 *
 * ls -l   126584
 *
 * Calculation using the values also reported by readelf -h:
 * Start of section headers	e_shoff		124728
 * Size of section headers		e_shentsize	64
 * Number of section headers	e_shnum		29
 *
 * e_shoff + ( e_shentsize * e_shnum ) =	126584
 */
ssize_t appimage_get_elf_size(const char* fname) __attribute__ ((deprecated));


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
