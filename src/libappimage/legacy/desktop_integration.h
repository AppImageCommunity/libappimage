#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions related to the desktop integration of AppImages.
 */

/**
 * Create a temporary dir to extract the AppImage Desktop Integration relate files such as:
 * - .Desktop file
 * - .AppIcon
 * - usr/share/icons/...
 *
 * @param appimage_path
 * @return path to the temporary dir
 */
char* desktop_integration_create_tempdir();

/**
 * Extract the AppImage Desktop Integration relate files such as:
 * - .Desktop file
 * - .AppIcon
 * - usr/share/icons/...
 *
 * @param appimage_path
 * @param tempdir_path
 */
void desktop_integration_extract_relevant_files(const char* appimage_path, const char* tempdir_path);


/**
 * Modifies the fields on the .Desktop file to make them point to the expected locations of the desktop integration
 * files.
 * Fields modified:
 * - Exec: will point to the AppImage
 * - TryExcec: will point to the AppImage
 * - Icon: Will point to the expected path of the application icon in $XDG_DATA_HOME/icons/.../apps/..
 * @param appimage_path
 * @param tempdir_path
 */
bool desktop_integration_modify_desktop_file(const char* appimage_path, const char* tempdir_path, const char* md5);


/**
 * Move .desktop file and application icons from <tempdir_path>/share to the $XDG_DATA_HOME
 * @param tempdir_path
 * @param user_data_dir
 * @param md5sum
 */
bool desktop_integration_move_files_to_user_data_dir(const char* tempdir_path, const char* user_data_dir,
                                                     const char* md5sum);

/**
 * Remove recusively remaining files at the temporary dir and the dir itself
 * @param dir_path
 */
void desktop_integration_remove_tempdir(const char* dir_path);

#ifdef __cplusplus
}
#endif
