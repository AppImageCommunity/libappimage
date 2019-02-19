#pragma once

// system includes
#include <glib.h>
#include <stdbool.h>

/* AppImage generic handler calback to be used in algorithms */
typedef void (*traverse_cb)(void* handler, void* entry_data, void* user_data);

/* AppImage generic handler to be used in algorithms */
struct appimage_handler {
    const gchar* path;

    char* (*get_file_name)(struct appimage_handler* handler, void* entry);

    void (*extract_file)(struct appimage_handler* handler, void* entry, const char* target);

    bool (*read_file_into_new_buffer)(struct appimage_handler* handler, void* entry, char** buffer, unsigned long* buffer_size);

    char* (*get_file_link)(struct appimage_handler* handler, void* entry);

    void (*traverse)(struct appimage_handler* handler, traverse_cb command, void* user_data);

    void* cache;
    bool is_open;

    // for debugging purposes
    int type;
} typedef appimage_handler;

/*
 * appimage_handler functions
 */

// constructor
appimage_handler create_appimage_handler(const char* const path);

/*
 * utility functions
 */
bool is_handler_valid(const appimage_handler* handler);

void mk_base_dir(const char *target);

/*
 * dummy fallback callbacks
 */
void dummy_traverse_func(appimage_handler* handler, traverse_cb command, void* data);

char* dummy_get_file_name(appimage_handler* handler, void* data);

void dummy_extract_file(struct appimage_handler* handler, void* data, char* target);

