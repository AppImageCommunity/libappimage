#pragma once

#include <glib.h>
#include <stdbool.h>

/* AppImage generic handler calback to be used in algorithms */
typedef void (*traverse_cb)(void *handler, void *entry_data, void *user_data);

/* AppImage generic handler to be used in algorithms */
struct appimage_handler
{
    const gchar *path;
    char* (*get_file_name) (struct appimage_handler *handler, void *entry);
    void (*extract_file) (struct appimage_handler *handler, void *entry, const char *target);

    void (*traverse)(struct appimage_handler *handler, traverse_cb command, void *user_data);

    void *cache;
    bool is_open;
} typedef appimage_handler;
