/**
 * Implementation of the C interface functions
 */

// local
#include <appimage/core/AppImage.h>

using namespace appimage::core;

extern "C" {

/* Check if a file is an AppImage. Returns the image type if it is, or -1 if it isn't */
int appimage_get_type(const char* path, bool verbose) {
    try {
        AppImage appImage(path);
        return appImage.getFormat();
    } catch (...) {
        return -1;
    }
}

}
