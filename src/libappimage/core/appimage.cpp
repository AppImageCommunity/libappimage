#include <iostream>

#include "utils/MagicBytesChecker.h"
#include "core/exceptions.h"
#include "appimage.h"

using namespace appimage;

core::appimage::appimage(const std::string& path) : path(path), format(getFormat(path)) {
    if (format == UNKNOWN)
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
        return TYPE_2;
    }

    return UNKNOWN;
}

const std::string& core::appimage::getPath() const {
    return path;
}

core::FORMAT core::appimage::getFormat() const {
    return format;
}

core::appimage::~appimage() {}

core::files_iterator core::appimage::files() {
    return files_iterator(path, format);
}
