// local
#include <appimage/appimage.h>

extern "C" {
int appimage_type1_is_terminal_app(const char* path) {
    return appimage_is_terminal_app(path);
} ;

int appimage_type2_is_terminal_app(const char* path) {
    return appimage_is_terminal_app(path);
} ;

#ifdef LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
/* Register a type 1 AppImage in the system
 * DEPRECATED, it should be removed ASAP
 * */
bool appimage_type1_register_in_system(const char* path, bool verbose) {
    return appimage_register_in_system(path, verbose) == 0;
}

/* Register a type 2 AppImage in the system
 * DEPRECATED it should be removed ASAP
 * */
bool appimage_type2_register_in_system(const char* path, bool verbose) {
    return appimage_register_in_system(path, verbose) == 0;
}

int appimage_type1_shall_not_be_integrated(const char* path) {
    return appimage_shall_not_be_integrated(path);
}

int appimage_type2_shall_not_be_integrated(const char* path) {
    return appimage_shall_not_be_integrated(path);
}

#endif // LIBAPPIMAGE_DESKTOP_INTEGRATION_ENABLED
}
