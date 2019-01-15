#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <stdbool.h>

// include header of shared library, which contains more appimage_ functions
#include <appimage/appimage_shared.h>

/* Return the md5 hash constructed according to
* https://specifications.freedesktop.org/thumbnail-spec/thumbnail-spec-latest.html#THUMBSAVE
* This can be used to identify files that are related to a given AppImage at a given location */
char *appimage_get_md5(char const* path);

/* Check if a file is an AppImage. Returns the image type if it is, or -1 if it isn't */
int appimage_get_type(const char* path, bool verbose);

/*
 * Finds the desktop file of a registered AppImage and returns the path
 * Returns NULL if the desktop file can't be found, which should only occur when the AppImage hasn't been registered yet
 */
char* appimage_registered_desktop_file_path(const char* path, char* md5, bool verbose);

/* Extract a given file from the appimage following the symlinks until a concrete file is found */
void appimage_extract_file_following_symlinks(const char* appimage_file_path, const char* file_path, const char* target_file_path);

/* Read a given file from the AppImage into a freshly allocated buffer following symlinks
 * Buffer must be free()d after usage
 * */
bool appimage_read_file_into_buffer_following_symlinks(const char* appimage_file_path, const char* file_path, char** buffer, unsigned long* buf_size);


/* List files contained in the AppImage file.
 * Returns: a newly allocated char** ended at NULL. If no files ware found also is returned a {NULL}
 *
 * You should ALWAYS take care of releasing this using `appimage_string_list_free`.
 * */
char** appimage_list_files(const char* path);

/* Releases memory of a string list (a.k.a. list of pointers to char arrays allocated in heap memory). */
void appimage_string_list_free(char** list);

/*
 * Checks whether a type 1 AppImage's desktop file has set Terminal=true.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type1_is_terminal_app(const char* path);

/*
 * Checks whether a type 2 AppImage's desktop file has set Terminal=true.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_type2_is_terminal_app(const char* path);

/*
 * Checks whether an AppImage's desktop file has set Terminal=true.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_is_terminal_app(const char* path);

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION

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

/*
 * Checks whether an AppImage's desktop file has set X-AppImage-Version=false.
 * Useful to check whether the author of an AppImage doesn't want it to be integrated.
 *
 * Returns >0 if set, 0 if not set, <0 on errors.
 */
int appimage_shall_not_be_integrated(const char* path);

/*
 * Check whether an AppImage has been registered in the system
 */
bool appimage_is_registered_in_system(const char* path);

/* Register a type 1 AppImage in the system
 * DEPRECATED don't use in newly written code. Use appimage_is_registered_in_system instead.
 * */
bool appimage_type1_register_in_system(const char *path, bool verbose);

/* Register a type 2 AppImage in the system
 * DEPRECATED don't use in newly written code. Use appimage_is_registered_in_system instead.
 * */
bool appimage_type2_register_in_system(const char *path, bool verbose);

/*
 * Register an AppImage in the system
 * Returns 0 on success, non-0 otherwise.
 */
int appimage_register_in_system(const char *path, bool verbose);

/* Unregister an AppImage in the system */
int appimage_unregister_in_system(const char *path, bool verbose);


#ifdef LIBAPPIMAGE_THUMBNAILER
/* Create AppImage thumbnail according to
 * https://specifications.freedesktop.org/thumbnail-spec/0.8.0/index.html
 */
void appimage_create_thumbnail(const char* appimage_file_path, bool verbose);
#endif // LIBAPPIMAGE_THUMBNAILER

#endif // LIBAPPIMAGE_DESKTOP_INTEGRATION


#ifdef __cplusplus
}
#endif
