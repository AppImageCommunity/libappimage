// system includes
#include "stdio.h"

// library includes
#include <appimage/appimage.h>

// local includes
#include "appimage_handler.h"
#include "appimage_handler_type1.h"
#include "appimage_handler_type2.h"

/* Factory function for creating the right appimage handler for
 * a given file. */
appimage_handler create_appimage_handler(const char *const path) {
    int appimage_type = appimage_get_type(path, 0);

    appimage_handler handler;
#ifdef STANDALONE
    fprintf(stderr,"AppImage type: %d\n", appimage_type);
#endif
    switch (appimage_type) {
        case 1:
            handler = appimage_type_1_create_handler();
            break;
        case 2:
            handler = appimage_type_2_create_handler();
            break;
        default:
#ifdef STANDALONE
            fprintf(stderr,"Appimage type %d not supported yet\n", appimage_type);
#endif
            handler.traverse = dummy_traverse_func;
            break;
    }
    handler.path = path;
    handler.is_open = false;
    return handler;
}

/*
 * utility functions
 */
bool is_handler_valid(const appimage_handler *handler) {
    if (handler == NULL) {
#ifdef STANDALONE
        fprintf(stderr, "WARNING: Invalid handler found, you should take a look at this now!");
#endif
        return false;
    }

    return true;
}

void mk_base_dir(const char *target)
{
    gchar *dirname = g_path_get_dirname(target);
    if(g_mkdir_with_parents(dirname, 0755)) {
#ifdef STANDALONE
        fprintf(stderr, "Could not create directory: %s\n", dirname);
#endif
    }

    g_free(dirname);
}

/*
 * Dummy fallback functions
 */
void dummy_traverse_func(appimage_handler *handler, traverse_cb command, void *data) {
    (void) handler;
    (void) command;
    (void) data;

    fprintf(stderr, "Called %s\n", __FUNCTION__);
}

char* dummy_get_file_name (appimage_handler *handler, void *data) {
    fprintf(stderr, "Called %s\n", __FUNCTION__);
}

void dummy_extract_file(struct appimage_handler *handler, void *data, char *target) {
    fprintf(stderr, "Called %s\n", __FUNCTION__);
}