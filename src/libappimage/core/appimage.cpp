#include <iostream>

#include "utils/MagicBytesChecker.h"
#include "core/exceptions.h"
#include "appimage.h"

using namespace appimage;

/**
 * Implementation of the opaque pointer patter for the appimage class
 * see https://en.wikipedia.org/wiki/Opaque_pointer
 */
struct core::appimage::appimage_priv {
    std::string path;
    FORMAT format;
};

core::appimage::appimage(const std::string& path) : d_ptr(new appimage_priv()) {
    d_ptr->path = path;
    d_ptr->format = getFormat(path);

    if (d_ptr->format == UNKNOWN)
        throw core::AppImageError("Unknown AppImage format");
}

core::FORMAT core::appimage::getFormat(const std::string& path) {
    MagicBytesChecker magicBytesChecker(path);
    if (magicBytesChecker.hasAppImageType1Signature())
        return TYPE_1;

    if (magicBytesChecker.hasAppImageType2Signature())
        return TYPE_2;

    if (magicBytesChecker.hasIso9660Signature() && magicBytesChecker.hasElfSignature()) {
        std::cerr << "WARNING: " << path << " seems to be a Type 1 AppImage without magic bytes." << std::endl;
        return TYPE_1;
    }

    return UNKNOWN;
}

const std::string& core::appimage::getPath() const {
    return d_ptr->path;
}

core::FORMAT core::appimage::getFormat() const {
    return d_ptr->format;
}

core::appimage::~appimage() {}

core::files_iterator core::appimage::files() {
    return files_iterator(d_ptr->path, d_ptr->format);
}
